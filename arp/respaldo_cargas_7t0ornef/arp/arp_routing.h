#ifndef EMILI_ARP_ROUTING_H
#define EMILI_ARP_ROUTING_H

#include "arp_tasks.h"

#include <map>
#include <string>
#include <vector>

namespace arp {

// Resultado de consultar un camino mínimo.
struct PathResult {
    bool reachable;
    double cost;

    // Identificadores originales de nodos y calles.
    std::vector<int> nodeIds;
    std::vector<int> linkIds;
};

// Calcula caminos sobre todas las calles de la instancia.
// Conserva el árbol del último origen consultado.
class ShortestPaths {
public:
    explicit ShortestPaths(const ArpInstance& instance);

    int indexOf(int nodeId) const;

    // El contenido cambia al consultar otro origen.
    const std::vector<double>& distancesFrom(int source);

    // Distancias desde todos los nodos hasta target.
    std::vector<double> distancesTo(int target) const;

    double distance(int source, int target);

    PathResult path(int source, int target);

private:
    std::vector<int> nodes_;
    std::map<int, int> index_;
    std::vector<std::vector<Step>> graph_;

    int source_ = -1;

    std::vector<double> distances_;
    std::vector<int> previous_;
    std::vector<int> previousLink_;
};

// Parámetros operativos del problema.
struct RoutingProblem {
    int startNode;
    int endNode;

    double capacity;

    // Carga de cada servicio, identificada por su ID.
    std::map<int, double> demand;
};

// Si start o end son -1, se utiliza el único nodo declarado.
// Si no existe un único nodo, debe seleccionarse explícitamente.
RoutingProblem makeProblem(
    const ArpInstance& instance,
    double capacity,
    double defaultDemand,
    const std::map<int, double>& overrides = {},
    int start = -1,
    int end = -1
);

// Movimiento físico por una calle.
struct Movement {
    int linkId;
    int from;
    int to;

    // true: se presta servicio.
    // false: se recorre en tránsito.
    bool service;
};

struct Route {
    // Servicios y orientaciones de la ruta.
    std::vector<TaskVisit> tasks;

    // Recorrido físico, incluyendo tránsito.
    std::vector<Movement> walk;

    double load = 0;
    double cost = 0;
};

struct RoutingSolution {
    std::vector<TaskVisit> giantTour;
    std::vector<Route> routes;

    double cost = 0;
};

struct ValidationResult {
    std::vector<std::string> errors;

    bool valid() const {
        return errors.empty();
    }
};

std::vector<TaskVisit> buildGiantTour(
    const ArpInstance& instance,
    const RoutingProblem& problem,
    ShortestPaths& paths
);

RoutingSolution splitByCapacity(
    const ArpInstance& instance,
    const RoutingProblem& problem,
    const std::vector<TaskVisit>& tour,
    ShortestPaths& paths
);

ValidationResult validateSolution(
    const ArpInstance& instance,
    const RoutingProblem& problem,
    const RoutingSolution& solution
);

} // namespace arp

#endif