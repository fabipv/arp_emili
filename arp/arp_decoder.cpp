#include "arp_decoder.h"

#include <algorithm>
#include <cmath>
#include <deque>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <utility>

namespace arp {

namespace {

// Politica numerica comun para comparaciones de carga y tiempo.
bool close(double a, double b) {
    return std::isfinite(a)
        && std::isfinite(b)
        && std::abs(a - b)
            <= 1e-9 * std::max(
                1.0,
                std::max(std::abs(a), std::abs(b))
            );
}

bool fits(double load, double capacity) {
    return std::isfinite(load)
        && (load <= capacity || close(load, capacity));
}

// Relaciona los ID originales con posiciones internas consecutivas.
struct Network {
    const ArpInstance& instance;
    const RoutingProblem& problem;

    TaskCatalog catalog;

    std::unordered_map<int, std::size_t> index;
    std::vector<double> demand;

    Network(
        const ArpInstance& data,
        const RoutingProblem& parameters
    )
        : instance(data),
          problem(parameters),
          catalog(data) {

        if (parameters.demand.size() != catalog.tasks().size()) {
            throw std::runtime_error(
                "Falta una carga por tarea requerida"
            );
        }

        // Reutiliza las comprobaciones existentes del problema.
        makeProblem(
            data,
            parameters.capacity,
            0,
            parameters.demand,
            parameters.startNode,
            parameters.endNode
        );

        demand.assign(data.links.size(), 0);

        for (std::size_t i = 0; i < data.links.size(); ++i) {
            const auto& link = data.links[i];

            if (link.id <= 0
                || !index.emplace(link.id, i).second
                || !std::isfinite(link.cost)
                || link.cost < 0) {

                throw std::runtime_error(
                    "ID o tiempo de calle invalido"
                );
            }

            if (link.required) {
                demand[i] = parameters.demand.at(link.id);
            }
        }
    }

    std::size_t slot(int id) const {
        const auto found = index.find(id);

        if (found == index.end()) {
            throw std::runtime_error(
                "Calle inexistente: " + std::to_string(id)
            );
        }

        return found->second;
    }

    bool permitted(
        const Link& link,
        int from,
        int to
    ) const {
        return (from == link.from && to == link.to)
            || (
                !link.directed
                && from == link.to
                && to == link.from
            );
    }

    void checkPriority(
        const std::vector<TaskVisit>& priority
    ) const {
        std::vector<unsigned char> seen(
            instance.links.size(),
            0
        );

        for (const auto& visit : priority) {
            catalog.resolve(visit);

            const auto i = slot(visit.taskId);

            if (seen[i]) {
                throw std::runtime_error(
                    "ID repetido en la prioridad"
                );
            }

            seen[i] = 1;
        }

        if (priority.size() != catalog.tasks().size()) {
            throw std::runtime_error(
                "La prioridad omite tareas requeridas"
            );
        }
    }
};

using PathPointer = std::shared_ptr<const PathResult>;

// Cache FIFO de caminos fisicos por par origen-destino.
// Limita entradas, no bytes.
class PathCache {
    ShortestPaths& paths_;
    std::size_t limit_;

    std::map<std::pair<int, int>, PathPointer> entries_;
    std::deque<std::pair<int, int>> order_;

public:
    PathCache(
        ShortestPaths& paths,
        std::size_t limit
    )
        : paths_(paths),
          limit_(limit) {
    }

    PathPointer get(int from, int to) {
        const auto key = std::make_pair(from, to);
        const auto found = entries_.find(key);

        if (found != entries_.end()) {
            return found->second;
        }

        auto result = std::make_shared<const PathResult>(
            paths_.path(from, to)
        );

        if (limit_ > 0) {
            if (entries_.size() == limit_) {
                entries_.erase(order_.front());
                order_.pop_front();
            }

            entries_.emplace(key, result);
            order_.push_back(key);
        }

        // El camino permanece disponible para el candidato actual
        // aunque una consulta posterior lo expulse del cache.
        return result;
    }
};

struct State {
    int node;
    double load = 0;

    std::vector<unsigned char> collected;
    std::size_t remaining;

    explicit State(const Network& net)
        : node(net.problem.startNode),
          collected(net.instance.links.size(), 0),
          remaining(net.catalog.tasks().size()) {
    }
};

// Simula varios caminos consecutivos con las mismas marcas temporales.
// Una prueba de extension y retorno usa una sola generacion.
class Probe {
    const Network& net_;

    std::vector<std::size_t> marks_;
    std::size_t generation_ = 0;

public:
    explicit Probe(const Network& net)
        : net_(net),
          marks_(net.instance.links.size(), 0) {
    }

    void begin() {
        if (generation_
            == std::numeric_limits<std::size_t>::max()) {

            std::fill(marks_.begin(), marks_.end(), 0);
            generation_ = 0;
        }

        ++generation_;
    }

    bool scan(
        const PathResult& path,
        const State& state,
        int& node,
        double& load,
        double& cost
    ) {
        if (!path.reachable) {
            return false;
        }

        if (path.nodeIds.size() != path.linkIds.size() + 1
            || path.nodeIds.front() != node) {

            throw std::runtime_error(
                "Camino con estructura invalida"
            );
        }

        double pathCost = 0;

        for (std::size_t k = 0; k < path.linkIds.size(); ++k) {
            const auto i = net_.slot(path.linkIds[k]);
            const auto& link = net_.instance.links[i];

            if (!net_.permitted(
                    link,
                    path.nodeIds[k],
                    path.nodeIds[k + 1]
                )) {

                throw std::runtime_error(
                    "Camino con sentido prohibido"
                );
            }

            if (link.required
                && !state.collected[i]
                && marks_[i] != generation_) {

                marks_[i] = generation_;
                load += net_.demand[i];
            }

            if (!fits(load, net_.problem.capacity)) {
                return false;
            }

            pathCost += link.cost;
        }

        if (!close(pathCost, path.cost)
            || !std::isfinite(cost + pathCost)) {

            throw std::runtime_error(
                "Tiempo de camino invalido"
            );
        }

        node = path.nodeIds.back();
        cost += pathCost;

        return true;
    }
};

// Confirma un camino previamente comprobado.
// Determina el servicio real utilizando las marcas globales.
void execute(
    const PathResult& path,
    const Network& net,
    State& state,
    Route& route
) {
    for (std::size_t k = 0; k < path.linkIds.size(); ++k) {
        const auto i = net.slot(path.linkIds[k]);
        const auto& link = net.instance.links[i];

        const int from = path.nodeIds[k];
        const int to = path.nodeIds[k + 1];

        const bool service =
            link.required && !state.collected[i];

        if (service) {
            state.collected[i] = 1;
            --state.remaining;

            state.load += net.demand[i];

            const bool reversed =
                from != link.from || to != link.to;

            route.tasks.push_back({
                link.id,
                reversed
            });
        }

        route.walk.push_back({
            link.id,
            from,
            to,
            service
        });

        route.cost += link.cost;
        state.node = to;
    }

    route.load = state.load;
}

struct Extension {
    PathPointer connection;
    PathPointer home;
    PathResult service;
};

// Criterio de seleccion:
// 1. Menor tiempo de conexion + travesia objetivo + retorno.
// 2. Orientacion preferida por el giant tour.
// 3. Orientacion directa.
//
// Cada orientacion prueba un camino minimo por conexion.
bool tryExtension(
    const TaskVisit& preferred,
    const Network& net,
    const State& state,
    PathCache& cache,
    Probe& probe,
    Extension& best
) {
    auto bestScore = std::make_tuple(
        std::numeric_limits<double>::infinity(),
        true,
        true
    );

    bool found = false;

    for (const auto& task
         : net.catalog.orientations(preferred.taskId)) {

        Extension candidate;

        candidate.connection = cache.get(
            state.node,
            task.from
        );

        candidate.home = cache.get(
            task.to,
            net.problem.endNode
        );

        candidate.service = {
            true,
            task.cost,
            {task.from, task.to},
            {task.taskId}
        };

        int node = state.node;
        double load = state.load;
        double cost = 0;

        probe.begin();

        if (!probe.scan(
                *candidate.connection,
                state,
                node,
                load,
                cost
            )
            || !probe.scan(
                candidate.service,
                state,
                node,
                load,
                cost
            )
            || !probe.scan(
                *candidate.home,
                state,
                node,
                load,
                cost
            )) {

            continue;
        }

        const auto score = std::make_tuple(
            cost,
            task.reversed != preferred.reversed,
            task.reversed
        );

        if (score < bestScore) {
            bestScore = score;
            best = std::move(candidate);
            found = true;
        }
    }

    return found;
}

} // namespace

DecodedSolution decodeFirstTraversal(
    const ArpInstance& instance,
    const RoutingProblem& problem,
    const std::vector<TaskVisit>& priority,
    ShortestPaths& paths,
    std::size_t cacheEntries
) {
    const Network net(instance, problem);
    net.checkPriority(priority);

    PathCache cache(paths, cacheEntries);
    Probe probe(net);
    State state(net);

    DecodedSolution result;
    result.priorityTour = priority;

    std::size_t cursor = 0;

    while (state.remaining > 0) {
        state.node = problem.startNode;
        state.load = 0;

        const auto before = state.remaining;

        Route route;
        PathPointer home;

        while (cursor < priority.size()) {
            if (state.collected[
                    net.slot(priority[cursor].taskId)
                ]) {

                ++cursor;
                continue;
            }

            Extension extension;

            if (!tryExtension(
                    priority[cursor],
                    net,
                    state,
                    cache,
                    probe,
                    extension
                )) {

                break;
            }

            execute(
                *extension.connection,
                net,
                state,
                route
            );

            execute(
                extension.service,
                net,
                state,
                route
            );

            home = extension.home;
            ++cursor;
        }

        // Despues de una extension aceptada, home conserva
        // el camino de retorno comprobado desde su salida.
        //
        // Si la ruta aun no tiene extensiones, se prueba s->f.
        // Ese recorrido tambien puede recoger tareas pendientes.
        if (!home) {
            home = cache.get(
                state.node,
                problem.endNode
            );
        }

        int node = state.node;
        double load = state.load;
        double cost = 0;

        probe.begin();

        if (!probe.scan(
                *home,
                state,
                node,
                load,
                cost
            )) {

            result.failure =
                "FALLO_RETORNO desde nodo "
                + std::to_string(state.node);

            return result;
        }

        execute(*home, net, state, route);

        if (state.remaining == before) {
            result.failure =
                "FALLO_SIN_PROGRESO ante ID "
                + std::to_string(
                    priority.at(cursor).taskId
                );

            return result;
        }

        result.cost += route.cost;

        if (!std::isfinite(result.cost)) {
            throw std::runtime_error(
                "Desbordamiento del tiempo total"
            );
        }

        result.routes.push_back(std::move(route));
    }

    result.complete = true;

    return result;
}

ValidationResult validateFirstTraversal(
    const ArpInstance& instance,
    const RoutingProblem& problem,
    const DecodedSolution& solution
) {
    ValidationResult result;

    try {
        const Network net(instance, problem);
        net.checkPriority(solution.priorityTour);

        std::vector<unsigned char> seen(
            instance.links.size(),
            0
        );

        double total = 0;

        if (!solution.complete || !solution.failure.empty()) {
            result.errors.push_back(
                "Decodificacion incompleta: "
                + solution.failure
            );
        }

        for (std::size_t r = 0;
             r < solution.routes.size();
             ++r) {

            const auto& route = solution.routes[r];

            const std::string prefix =
                "Ruta " + std::to_string(r + 1) + ": ";

            const auto error = [&](const std::string& text) {
                result.errors.push_back(prefix + text);
            };

            int node = problem.startNode;
            double load = 0;
            double cost = 0;

            std::size_t servicePosition = 0;

            for (const auto& move : route.walk) {
                const auto i = net.slot(move.linkId);
                const auto& link = instance.links[i];

                if (move.from != node) {
                    error("recorrido discontinuo");
                }

                if (!net.permitted(
                        link,
                        move.from,
                        move.to
                    )) {

                    error("sentido prohibido");
                }

                node = move.to;
                cost += link.cost;

                const bool expectedService =
                    link.required && !seen[i];

                if (move.service != expectedService) {
                    error(
                        "marca de servicio incorrecta en ID "
                        + std::to_string(link.id)
                    );
                }

                // Se deduce la recoleccion a partir del recorrido,
                // independientemente del indicador declarado.
                if (expectedService) {
                    seen[i] = 1;
                    load += net.demand[i];

                    const bool reversed =
                        move.from != link.from
                        || move.to != link.to;

                    if (servicePosition >= route.tasks.size()
                        || route.tasks[servicePosition].taskId
                            != link.id
                        || route.tasks[servicePosition].reversed
                            != reversed) {

                        error(
                            "lista de servicios distinta del recorrido"
                        );
                    }

                    ++servicePosition;
                }

                if (!fits(load, problem.capacity)) {
                    error("capacidad excedida");
                }
            }

            if (node != problem.endNode) {
                error("termino incorrecto");
            }

            if (servicePosition == 0
                || servicePosition != route.tasks.size()) {

                error(
                    "ruta sin progreso o lista de servicios incorrecta"
                );
            }

            if (!close(load, route.load)
                || !close(cost, route.cost)) {

                error(
                    "carga o tiempo declarado incorrecto"
                );
            }

            total += cost;
        }

        for (std::size_t i = 0;
             i < instance.links.size();
             ++i) {

            if (instance.links[i].required && !seen[i]) {
                result.errors.push_back(
                    "Tarea pendiente: "
                    + std::to_string(instance.links[i].id)
                );
            }
        }

        if (!close(total, solution.cost)) {
            result.errors.push_back(
                "Tiempo total incorrecto"
            );
        }

    } catch (const std::exception& error) {
        result.errors.push_back(error.what());
    }

    return result;
}

} // namespace arp