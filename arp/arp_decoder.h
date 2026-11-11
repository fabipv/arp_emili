#ifndef EMILI_ARP_DECODER_H
#define EMILI_ARP_DECODER_H

#include "arp_routing.h"
#include <cstddef>

namespace arp {

// La prioridad y el orden real de servicio pueden ser diferentes.
// Route.tasks conserva el orden real de recoleccion.
struct DecodedSolution {
    std::vector<TaskVisit> priorityTour;
    std::vector<Route> routes;

    double cost = 0;
    bool complete = false;
    std::string failure;
};

// cacheEntries limita el numero de caminos almacenados.
// Las marcas de recoleccion se calculan para cada candidato.
DecodedSolution decodeFirstTraversal(
    const ArpInstance& instance,
    const RoutingProblem& problem,
    const std::vector<TaskVisit>& priority,
    ShortestPaths& paths,
    std::size_t cacheEntries = 512
);

ValidationResult validateFirstTraversal(
    const ArpInstance& instance,
    const RoutingProblem& problem,
    const DecodedSolution& solution
);

} // namespace arp

#endif