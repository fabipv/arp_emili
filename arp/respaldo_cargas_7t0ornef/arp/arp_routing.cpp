#include "arp_routing.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <queue>
#include <set>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace arp {

namespace {

const double INF = std::numeric_limits<double>::infinity();

// Dijkstra sobre índices internos del grafo.
void dijkstra(
    const std::vector<std::vector<Step>>& graph,
    int source,
    std::vector<double>& distance,
    std::vector<int>& previous,
    std::vector<int>& previousLink
) {
    distance.assign(graph.size(), INF);
    previous.assign(graph.size(), -1);
    previousLink.assign(graph.size(), -1);

    using Entry = std::pair<double, int>;

    std::priority_queue<
        Entry,
        std::vector<Entry>,
        std::greater<Entry>
    > queue;

    distance.at(source) = 0;
    queue.push({0, source});

    while (!queue.empty()) {
        const Entry entry = queue.top();
        queue.pop();

        const int u = entry.second;

        // Descarta entradas antiguas de la cola.
        if (entry.first != distance[u]) {
            continue;
        }

        for (const auto& edge : graph[u]) {
            const double candidate = distance[u] + edge.cost;

            if (!std::isfinite(candidate)) {
                throw std::runtime_error(
                    "Desbordamiento al sumar costos"
                );
            }

            // La mejora estricta evita ciclos de predecesores
            // cuando existen calles de costo cero.
            if (candidate < distance[edge.toIndex]) {
                distance[edge.toIndex] = candidate;
                previous[edge.toIndex] = u;
                previousLink[edge.toIndex] = edge.linkId;

                queue.push({
                    candidate,
                    edge.toIndex
                });
            }
        }
    }
}

// Comparación de costos y cargas declaradas con tolerancia.
bool close(double a, double b) {
    return std::isfinite(a)
        && std::isfinite(b)
        && std::abs(a - b)
            <= 1e-9
                * std::max(
                    1.0,
                    std::max(std::abs(a), std::abs(b))
                );
}

// Comprueba los parámetros antes de construir rutas.
void checkProblem(
    const ArpInstance& instance,
    const RoutingProblem& p
) {
    if (instance.declaredRestrictions
        || !instance.restrictionRows.empty()) {

        throw std::runtime_error(
            "Esta version no interpreta restricciones adicionales"
        );
    }

    if (!std::isfinite(p.capacity) || p.capacity <= 0) {
        throw std::runtime_error(
            "La capacidad debe ser finita y positiva"
        );
    }

    for (int node : {p.startNode, p.endNode}) {
        if (std::find(
                instance.nodeIds.begin(),
                instance.nodeIds.end(),
                node
            ) == instance.nodeIds.end()) {

            throw std::runtime_error(
                "Nodo de inicio o termino inexistente"
            );
        }
    }

    if (!instance.startNodes.empty()
        && std::find(
            instance.startNodes.begin(),
            instance.startNodes.end(),
            p.startNode
        ) == instance.startNodes.end()) {

        throw std::runtime_error(
            "El inicio no pertenece a NODOS_INICIALES"
        );
    }

    if (!instance.endNodes.empty()
        && std::find(
            instance.endNodes.begin(),
            instance.endNodes.end(),
            p.endNode
        ) == instance.endNodes.end()) {

        throw std::runtime_error(
            "El termino no pertenece a NODOS_TERMINO"
        );
    }

    const TaskCatalog catalog(instance);

    if (p.demand.size() != catalog.tasks().size()) {
        throw std::runtime_error(
            "Debe existir una demanda por tarea requerida"
        );
    }

    for (const auto& task : catalog.tasks()) {
        const auto found = p.demand.find(task.id);

        if (found == p.demand.end()
            || !std::isfinite(found->second)
            || found->second < 0
            || found->second > p.capacity) {

            throw std::runtime_error(
                "Demanda ausente, invalida o mayor que Q: tarea "
                + std::to_string(task.id)
            );
        }
    }
}

// Comprueba cobertura y orientaciones del giant tour.
void checkTour(
    const TaskCatalog& catalog,
    const std::vector<TaskVisit>& tour
) {
    std::set<int> seen;

    for (const auto& visit : tour) {
        catalog.resolve(visit);

        if (!seen.insert(visit.taskId).second) {
            throw std::runtime_error(
                "Tarea repetida en giant tour"
            );
        }
    }

    if (seen.size() != catalog.tasks().size()) {
        throw std::runtime_error(
            "El giant tour omite tareas requeridas"
        );
    }
}

int selectNode(
    int selected,
    const std::vector<int>& declared,
    const char* option
) {
    if (selected >= 0) {
        return selected;
    }

    if (declared.size() != 1) {
        throw std::runtime_error(
            std::string("Debes indicar ")
            + option
            + ": no hay un unico nodo declarado"
        );
    }

    return declared.front();
}

// Agrega a una ruta el camino de tránsito entre dos nodos.
void connect(
    Route& route,
    int from,
    int to,
    ShortestPaths& paths
) {
    const PathResult path = paths.path(from, to);

    if (!path.reachable) {
        throw std::runtime_error(
            "Conexion inalcanzable: "
            + std::to_string(from)
            + " -> "
            + std::to_string(to)
        );
    }

    for (std::size_t i = 0; i < path.linkIds.size(); ++i) {
        route.walk.push_back({
            path.linkIds[i],
            path.nodeIds[i],
            path.nodeIds[i + 1],
            false
        });
    }

    // El tránsito aumenta el costo, pero no la carga.
    route.cost += path.cost;

    if (!std::isfinite(route.cost)) {
        throw std::runtime_error(
            "Costo fuera de rango"
        );
    }
}

} // namespace

ShortestPaths::ShortestPaths(const ArpInstance& instance)
    : nodes_(instance.nodeIds) {

    std::set<int> ids;

    for (const auto& link : instance.links) {
        if (link.id <= 0
            || !ids.insert(link.id).second
            || !std::isfinite(link.cost)
            || link.cost < 0) {

            throw std::runtime_error(
                "ID o costo de calle invalido"
            );
        }
    }

    for (std::size_t i = 0; i < nodes_.size(); ++i) {
        if (!index_.emplace(
                nodes_[i],
                static_cast<int>(i)
            ).second) {

            throw std::runtime_error(
                "Nodo repetido en nodeIds"
            );
        }
    }

    // Incluye calles requeridas y no requeridas.
    graph_ = instance.adjacency();
}

int ShortestPaths::indexOf(int nodeId) const {
    const auto found = index_.find(nodeId);

    if (found == index_.end()) {
        throw std::runtime_error(
            "Nodo inexistente: " + std::to_string(nodeId)
        );
    }

    return found->second;
}

const std::vector<double>& ShortestPaths::distancesFrom(
    int source
) {
    const int index = indexOf(source);

    if (source_ != index) {
        source_ = -1;

        dijkstra(
            graph_,
            index,
            distances_,
            previous_,
            previousLink_
        );

        source_ = index;
    }

    return distances_;
}

std::vector<double> ShortestPaths::distancesTo(
    int target
) const {
    // Invierte los movimientos para calcular todas las
    // distancias hacia un mismo destino con un solo Dijkstra.
    std::vector<std::vector<Step>> reverse(graph_.size());

    for (std::size_t u = 0; u < graph_.size(); ++u) {
        for (const auto& edge : graph_[u]) {
            reverse[edge.toIndex].push_back({
                static_cast<int>(u),
                edge.linkId,
                edge.cost
            });
        }
    }

    std::vector<double> distance;
    std::vector<int> previous;
    std::vector<int> previousLink;

    dijkstra(
        reverse,
        indexOf(target),
        distance,
        previous,
        previousLink
    );

    return distance;
}

double ShortestPaths::distance(
    int source,
    int target
) {
    const int destination = indexOf(target);

    return distancesFrom(source).at(destination);
}

PathResult ShortestPaths::path(
    int source,
    int target
) {
    const double cost = distance(source, target);

    PathResult result{
        std::isfinite(cost),
        cost,
        {},
        {}
    };

    if (!result.reachable) {
        return result;
    }

    int current = indexOf(target);
    const int origin = indexOf(source);

    result.nodeIds.push_back(nodes_[current]);

    while (current != origin) {
        if (current < 0
            || result.linkIds.size() >= nodes_.size()) {

            throw std::runtime_error(
                "Cadena de predecesores invalida"
            );
        }

        result.linkIds.push_back(
            previousLink_.at(current)
        );

        current = previous_.at(current);

        result.nodeIds.push_back(
            nodes_.at(current)
        );
    }

    std::reverse(
        result.nodeIds.begin(),
        result.nodeIds.end()
    );

    std::reverse(
        result.linkIds.begin(),
        result.linkIds.end()
    );

    return result;
}

RoutingProblem makeProblem(
    const ArpInstance& instance,
    double capacity,
    double defaultDemand,
    const std::map<int, double>& overrides,
    int start,
    int end
) {
    if (!std::isfinite(defaultDemand)
        || defaultDemand < 0) {

        throw std::runtime_error(
            "Debes indicar una demanda base finita y no negativa"
        );
    }

    RoutingProblem p;

    p.startNode = selectNode(
        start,
        instance.startNodes,
        "--start"
    );

    p.endNode = selectNode(
        end,
        instance.endNodes,
        "--end"
    );

    p.capacity = capacity;

    const TaskCatalog catalog(instance);

    for (const auto& task : catalog.tasks()) {
        p.demand[task.id] = defaultDemand;
    }

    for (const auto& item : overrides) {
        catalog.byId(item.first);
        p.demand[item.first] = item.second;
    }

    checkProblem(instance, p);

    return p;
}

std::vector<TaskVisit> buildGiantTour(
    const ArpInstance& instance,
    const RoutingProblem& p,
    ShortestPaths& paths
) {
    checkProblem(instance, p);

    const TaskCatalog catalog(instance);

    const auto toEnd = paths.distancesTo(p.endNode);

    std::vector<std::vector<OrientedTask>> options;

    for (const auto& task : catalog.tasks()) {
        options.push_back(
            catalog.orientations(task.id)
        );
    }

    std::vector<bool> used(options.size(), false);
    std::vector<TaskVisit> tour;

    int current = p.startNode;

    while (tour.size() < options.size()) {
        const auto& distance =
            paths.distancesFrom(current);

        // Criterio:
        // 1. Menor conexión + servicio.
        // 2. Menor ID.
        // 3. Orientación directa.
        std::tuple<double, int, bool> best(
            INF,
            0,
            false
        );

        std::size_t selected = options.size();
        OrientedTask chosen{};

        for (std::size_t i = 0; i < options.size(); ++i) {
            if (used[i]) {
                continue;
            }

            for (const auto& option : options[i]) {
                const double entry = distance.at(
                    paths.indexOf(option.from)
                );

                if (!std::isfinite(entry)
                    || !std::isfinite(
                        toEnd.at(
                            paths.indexOf(option.to)
                        )
                    )) {

                    continue;
                }

                const auto score = std::make_tuple(
                    entry + option.cost,
                    option.taskId,
                    option.reversed
                );

                if (score < best) {
                    best = score;
                    selected = i;
                    chosen = option;
                }
            }
        }

        if (selected == options.size()) {
            throw std::runtime_error(
                "El constructor no pudo completar el giant tour; "
                "esto no demuestra que la instancia sea inviable"
            );
        }

        used[selected] = true;

        tour.push_back({
            chosen.taskId,
            chosen.reversed
        });

        current = chosen.to;
    }

    return tour;
}

RoutingSolution splitByCapacity(
    const ArpInstance& instance,
    const RoutingProblem& p,
    const std::vector<TaskVisit>& tour,
    ShortestPaths& paths
) {
    checkProblem(instance, p);

    const TaskCatalog catalog(instance);
    checkTour(catalog, tour);

    RoutingSolution solution;
    solution.giantTour = tour;

    Route route;
    int current = p.startNode;

    for (const auto& visit : tour) {
        const auto task = catalog.resolve(visit);
        const double demand =
            p.demand.at(task.taskId);

        // Se permite carga igual a Q.
        // Se corta antes de agregar una tarea que exceda Q.
        if (!route.tasks.empty()
            && route.load + demand > p.capacity) {

            connect(
                route,
                current,
                p.endNode,
                paths
            );

            solution.cost += route.cost;
            solution.routes.push_back(
                std::move(route)
            );

            route = Route{};
            current = p.startNode;
        }

        connect(
            route,
            current,
            task.from,
            paths
        );

        // Agrega el servicio explícitamente.
        route.walk.push_back({
            task.taskId,
            task.from,
            task.to,
            true
        });

        route.tasks.push_back(visit);
        route.load += demand;
        route.cost += task.cost;

        current = task.to;
    }

    if (!route.tasks.empty()) {
        connect(
            route,
            current,
            p.endNode,
            paths
        );

        solution.cost += route.cost;
        solution.routes.push_back(
            std::move(route)
        );
    }

    if (!std::isfinite(solution.cost)) {
        throw std::runtime_error(
            "Costo total fuera de rango"
        );
    }

    return solution;
}

ValidationResult validateSolution(
    const ArpInstance& instance,
    const RoutingProblem& p,
    const RoutingSolution& solution
) {
    ValidationResult result;

    const auto error = [&](const std::string& text) {
        result.errors.push_back(text);
    };

    try {
        checkProblem(instance, p);

        const TaskCatalog catalog(instance);

        try {
            checkTour(catalog, solution.giantTour);
        } catch (const std::exception& e) {
            error(e.what());
        }

        std::map<int, const Link*> links;

        for (const auto& link : instance.links) {
            if (!std::isfinite(link.cost)
                || link.cost < 0
                || link.id <= 0
                || !links.emplace(
                    link.id,
                    &link
                ).second) {

                throw std::runtime_error(
                    "ID o costo de calle invalido"
                );
            }
        }

        std::map<int, int> served;

        std::size_t position = 0;
        double totalCost = 0;

        for (std::size_t r = 0;
             r < solution.routes.size();
             ++r) {

            const auto& route = solution.routes[r];

            const std::string prefix =
                "Ruta " + std::to_string(r + 1) + ": ";

            if (route.tasks.empty()) {
                error(prefix + "ruta vacia");
            }

            // Verifica que el corte conserve el giant tour.
            for (const auto& visit : route.tasks) {
                try {
                    catalog.resolve(visit);
                } catch (const std::exception& e) {
                    error(prefix + e.what());
                }

                if (position >= solution.giantTour.size()
                    || visit.taskId
                        != solution.giantTour[position].taskId
                    || visit.reversed
                        != solution.giantTour[position].reversed) {

                    error(
                        prefix
                        + "no conserva el orden del giant tour"
                    );
                }

                ++position;
            }

            int current = p.startNode;
            std::size_t servicePosition = 0;

            double load = 0;
            double cost = 0;

            // Revisa el recorrido físico utilizando las calles
            // originales, sin reconstruirlo con Dijkstra.
            for (const auto& move : route.walk) {
                if (move.from != current) {
                    error(
                        prefix + "recorrido discontinuo"
                    );
                }

                current = move.to;

                const auto found =
                    links.find(move.linkId);

                if (found == links.end()) {
                    error(
                        prefix + "calle inexistente"
                    );
                    continue;
                }

                const Link& link = *found->second;

                const bool forward =
                    move.from == link.from
                    && move.to == link.to;

                const bool backward =
                    !link.directed
                    && move.from == link.to
                    && move.to == link.from;

                if (!forward && !backward) {
                    error(
                        prefix + "movimiento no permitido"
                    );
                }

                cost += link.cost;

                // Un movimiento en tránsito no atiende tareas.
                if (!move.service) {
                    continue;
                }

                if (!link.required) {
                    error(
                        prefix
                        + "servicio en calle no requerida"
                    );
                }

                ++served[link.id];

                const auto demand =
                    p.demand.find(link.id);

                if (demand != p.demand.end()) {
                    load += demand->second;
                }

                if (load > p.capacity) {
                    error(
                        prefix + "capacidad excedida"
                    );
                }

                if (servicePosition >= route.tasks.size()) {
                    error(
                        prefix + "servicio extra"
                    );
                } else {
                    try {
                        const auto expected =
                            catalog.resolve(
                                route.tasks[servicePosition]
                            );

                        if (expected.taskId != move.linkId
                            || expected.from != move.from
                            || expected.to != move.to) {

                            error(
                                prefix
                                + "servicio distinto del declarado"
                            );
                        }
                    } catch (const std::exception& e) {
                        error(prefix + e.what());
                    }
                }

                ++servicePosition;
            }

            if (current != p.endNode) {
                error(
                    prefix + "termino incorrecto"
                );
            }

            if (servicePosition != route.tasks.size()) {
                error(
                    prefix + "faltan servicios fisicos"
                );
            }

            if (!close(load, route.load)) {
                error(
                    prefix + "carga declarada incorrecta"
                );
            }

            if (!close(cost, route.cost)) {
                error(
                    prefix + "costo declarado incorrecto"
                );
            }

            totalCost += cost;
        }

        if (position != solution.giantTour.size()) {
            error(
                "Las rutas omiten parte del giant tour"
            );
        }

        for (const auto& task : catalog.tasks()) {
            if (served[task.id] != 1) {
                error(
                    "La tarea "
                    + std::to_string(task.id)
                    + " no se atiende exactamente una vez"
                );
            }
        }

        if (!close(totalCost, solution.cost)) {
            error(
                "Costo total incorrecto"
            );
        }

    } catch (const std::exception& e) {
        error(e.what());
    }

    return result;
}

} // namespace arp
