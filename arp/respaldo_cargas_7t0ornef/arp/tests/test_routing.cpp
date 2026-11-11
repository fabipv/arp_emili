#include "arp_routing.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <set>
#include <stdexcept>

namespace {

void check(bool value, const char* message) {
    if (!value) {
        throw std::runtime_error(message);
    }
}

template<class F>
void rejects(F action, const char* message) {
    bool failed = false;

    try {
        action();
    } catch (const std::exception&) {
        failed = true;
    }

    check(failed, message);
}

arp::ArpInstance instance() {
    arp::ArpInstance i;

    i.startNodes = {10};
    i.endNodes = {10};

    i.nodeIds = {10, 20, 30, 40, 99};

    i.links = {
        {1, 10, 20, false, 4, true, 1, 1},
        {2, 20, 30, true, 2, true, 1, 2},

        // Calle paralela opcional más barata.
        {3, 10, 20, false, 1, false, 0, 3},

        {4, 30, 10, true, 3, false, 0, 4},

        // Servicio de costo cero.
        {5, 30, 40, false, 0, true, 1, 5},

        // Bucle de costo cero.
        {6, 40, 40, false, 0, false, 0, 6}
    };

    return i;
}

} // namespace

int main() {
    try {
        auto i = instance();

        arp::ShortestPaths paths(i);

        auto path = paths.path(10, 30);

        check(
            path.reachable && path.cost == 3,
            "Dijkstra / calle paralela"
        );

        check(
            path.linkIds == std::vector<int>({3, 2}),
            "Reconstruccion por ID"
        );

        check(
            paths.distance(30, 10) == 3,
            "Sentidos dirigidos"
        );

        path = paths.path(40, 40);

        check(
            path.cost == 0
                && path.nodeIds == std::vector<int>({40})
                && path.linkIds.empty(),
            "Origen igual al destino"
        );

        path = paths.path(10, 99);

        check(
            !path.reachable
                && std::isinf(path.cost)
                && path.nodeIds.empty(),
            "Inalcanzable"
        );

        rejects(
            [&] { paths.path(10, 999); },
            "Nodo inexistente"
        );

        check(
            paths.path(10, 40).cost == 3,
            "Costos cero sin ciclos de predecesores"
        );

        // Prueba con cargas uniformes de control.
        auto p = arp::makeProblem(i, 2, 1);

        auto tour = arp::buildGiantTour(i, p, paths);

        auto solution = arp::splitByCapacity(
            i, p, tour, paths
        );

        check(
            solution.routes.size() == 2,
            "Division por capacidad"
        );

        check(
            solution.routes[0].load == 2,
            "Se admite carga igual a Q"
        );

        check(
            arp::validateSolution(i, p, solution).valid(),
            "Solucion correcta"
        );

        check(
            solution.giantTour.size() == 3,
            "Solo servicios requeridos"
        );

        const auto invalid = [&](
            arp::RoutingSolution changed,
            const char* message
        ) {
            check(
                !arp::validateSolution(
                    i, p, changed
                ).valid(),
                message
            );
        };

        auto changed = solution;
        changed.routes[0].walk[0].from = 99;
        invalid(changed, "Discontinuidad");

        changed = solution;
        changed.routes[0].walk[0].linkId = 999;
        invalid(changed, "ID de calle inexistente");

        changed = solution;
        changed.routes[0].cost += 1;
        invalid(changed, "Costo de ruta incorrecto");

        changed = solution;
        changed.cost += 1;
        invalid(changed, "Costo total incorrecto");

        changed = solution;
        changed.routes[0].load += 1;
        invalid(changed, "Carga declarada incorrecta");

        changed = solution;
        changed.routes.push_back(arp::Route{});
        invalid(changed, "Ruta vacia");

        changed = solution;
        changed.routes.pop_back();
        invalid(changed, "Servicio omitido");

        changed = solution;
        changed.routes.push_back(solution.routes[0]);
        invalid(changed, "Servicio duplicado");

        changed = solution;
        changed.giantTour[0].taskId = 3;
        invalid(changed, "Calle opcional como tarea");

        changed = solution;

        for (auto& visit : changed.giantTour) {
            if (visit.taskId == 2) {
                visit.reversed = true;
            }
        }

        invalid(changed, "Inversion de uni");

        changed = solution;

        for (auto& route : changed.routes) {
            for (auto& move : route.walk) {
                if (move.linkId == 2) {
                    const int u = move.from;
                    move.from = move.to;
                    move.to = u;
                }
            }
        }

        invalid(changed, "Movimiento contrario a uni");

        changed = solution;
        changed.routes[0].walk.back().to = 99;
        invalid(changed, "Termino incorrecto");

        auto smaller = p;
        smaller.capacity = 1;

        check(
            !arp::validateSolution(
                i, smaller, solution
            ).valid(),
            "Exceso real de capacidad"
        );

        auto extra = tour;
        extra.push_back(tour.front());

        rejects(
            [&] {
                arp::splitByCapacity(i, p, extra, paths);
            },
            "Tour duplicado"
        );

        rejects(
            [&] { arp::makeProblem(i, 0, 1); },
            "Q cero"
        );

        rejects(
            [&] { arp::makeProblem(i, 1, 2); },
            "Demanda mayor que Q"
        );

        rejects(
            [&] {
                arp::makeProblem(i, 2, 1, {{3, 1}});
            },
            "Demanda para calle opcional"
        );

        auto heterogeneous = arp::makeProblem(
            i, 3, 1, {{1, 2}}
        );

        auto heteroSolution = arp::splitByCapacity(
            i, heterogeneous, tour, paths
        );

        check(
            arp::validateSolution(
                i, heterogeneous, heteroSolution
            ).valid(),
            "Demandas diferentes"
        );

        auto zero = arp::makeProblem(i, 1, 0);

        check(
            arp::splitByCapacity(
                i, zero, tour, paths
            ).routes.size() == 1,
            "Demandas cero"
        );

        // Prueba de la regla carga = costo.
        std::map<int, double> costDemand;

        for (const auto& link : i.links) {
            if (link.required) {
                costDemand[link.id] = link.cost;
            }
        }

        auto costProblem = arp::makeProblem(
            i, 4, 0, costDemand
        );

        auto costSolution = arp::splitByCapacity(
            i, costProblem, tour, paths
        );

        check(
            arp::validateSolution(
                i, costProblem, costSolution
            ).valid(),
            "Costo como carga"
        );

        check(
            costSolution.routes.size() == 2,
            "Corte por costos de servicio"
        );

        check(
            costSolution.cost > 4,
            "El transito suma costo pero no carga"
        );

        changed = solution;
        changed.routes[0].walk[0].service = true;

        invalid(
            changed,
            "Servicio indebido en calle opcional"
        );

        changed = solution;

        changed.cost =
            std::numeric_limits<double>::quiet_NaN();

        invalid(changed, "Costo NaN");

        auto disconnected = i;

        disconnected.links.push_back({
            7, 99, 99, true, 1, true, 1, 7
        });

        arp::ShortestPaths disconnectedPaths(
            disconnected
        );

        auto disconnectedProblem = arp::makeProblem(
            disconnected, 2, 1
        );

        rejects(
            [&] {
                arp::buildGiantTour(
                    disconnected,
                    disconnectedProblem,
                    disconnectedPaths
                );
            },
            "Servicio inalcanzable"
        );

        // Inicio y término diferentes.
        i.endNodes = {30};

        auto open = arp::makeProblem(i, 2, 1);
        auto openTour = arp::buildGiantTour(
            i, open, paths
        );

        check(
            arp::validateSolution(
                i,
                open,
                arp::splitByCapacity(
                    i, open, openTour, paths
                )
            ).valid(),
            "Inicio y termino distintos"
        );

        i.endNodes.clear();

        rejects(
            [&] { arp::makeProblem(i, 2, 1); },
            "Termino ausente requiere decision"
        );

        check(
            arp::makeProblem(
                i, 2, 1, {}, -1, 10
            ).endNode == 10,
            "Termino explicito"
        );

        i.declaredRestrictions = 1;

        rejects(
            [&] {
                arp::makeProblem(
                    i, 2, 1, {}, -1, 10
                );
            },
            "Restricciones no interpretadas"
        );

        i.declaredRestrictions = 0;
        i.links.clear();
        i.startNodes = {10};
        i.endNodes = {10};

        arp::ShortestPaths emptyPaths(i);

        auto emptyProblem = arp::makeProblem(
            i, 1, 0
        );

        auto empty = arp::splitByCapacity(
            i, emptyProblem, {}, emptyPaths
        );

        check(
            empty.routes.empty()
                && arp::validateSolution(
                    i, emptyProblem, empty
                ).valid(),
            "Sin tareas"
        );

        // Contraste de Dijkstra con Floyd-Warshall.
        std::mt19937 random(42);

        for (int trial = 0; trial < 12; ++trial) {
            arp::ArpInstance sample;

            const int n = 7;

            std::vector<std::vector<double>> expected(
                n,
                std::vector<double>(
                    n,
                    std::numeric_limits<double>::infinity()
                )
            );

            for (int u = 0; u < n; ++u) {
                sample.nodeIds.push_back(100 + 10 * u);
                expected[u][u] = 0;
            }

            for (int e = 0; e < 18; ++e) {
                const int u = random() % n;
                const int v = random() % n;
                const double w = random() % 8;
                const bool directed = random() % 2;

                sample.links.push_back({
                    e + 1,
                    100 + 10 * u,
                    100 + 10 * v,
                    directed,
                    w,
                    false,
                    0,
                    e + 1
                });

                expected[u][v] = std::min(
                    expected[u][v], w
                );

                if (!directed) {
                    expected[v][u] = std::min(
                        expected[v][u], w
                    );
                }
            }

            for (int k = 0; k < n; ++k) {
                for (int u = 0; u < n; ++u) {
                    for (int v = 0; v < n; ++v) {
                        expected[u][v] = std::min(
                            expected[u][v],
                            expected[u][k] + expected[k][v]
                        );
                    }
                }
            }

            arp::ShortestPaths shortest(sample);

            for (int v = 0; v < n; ++v) {
                const auto reverse =
                    shortest.distancesTo(100 + 10 * v);

                for (int u = 0; u < n; ++u) {
                    const auto found = shortest.path(
                        100 + 10 * u,
                        100 + 10 * v
                    );

                    check(
                        found.cost == expected[u][v]
                            && reverse[u] == expected[u][v],
                        "Dijkstra vs Floyd"
                    );

                    if (!found.reachable) {
                        continue;
                    }

                    check(
                        found.nodeIds.front() == 100 + 10 * u
                            && found.nodeIds.back() == 100 + 10 * v,
                        "Extremos del camino reconstruido"
                    );

                    double sum = 0;

                    for (std::size_t s = 0;
                         s < found.linkIds.size();
                         ++s) {

                        const auto& edge =
                            sample.links.at(
                                found.linkIds[s] - 1
                            );

                        const int a = found.nodeIds[s];
                        const int b = found.nodeIds[s + 1];

                        check(
                            (a == edge.from && b == edge.to)
                                || (!edge.directed
                                    && a == edge.to
                                    && b == edge.from),
                            "Camino permitido"
                        );

                        sum += edge.cost;
                    }

                    check(
                        sum == found.cost,
                        "Costo del camino reconstruido"
                    );
                }
            }
        }

        std::cout
            << "OK: caminos, giant tour, capacidad "
            << "y validacion independiente\n";

        return 0;

    } catch (const std::exception& e) {
        std::cerr
            << "FALLO: "
            << e.what()
            << '\n';

        return 1;
    }
}
