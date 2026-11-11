#ifndef EMILI_ARP_ROUTING_H
#define EMILI_ARP_ROUTING_H

#include "arp_tasks.h"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace arp {

struct PathResult {
    bool reachable;
    double cost;

    std::vector<int> nodeIds;
    std::vector<int> linkIds;
};

class ShortestPaths {
public:
    explicit ShortestPaths(const ArpInstance& instance);

    int indexOf(int nodeId) const;

    const std::vector<double>& distancesFrom(int source);

    std::vector<double> distancesTo(int target) const;

    double distance(int source, int target);

    PathResult path(int source, int target);

    std::size_t dijkstraCalls() const {
        return dijkstraCalls_;
    }

    double dijkstraMilliseconds() const {
        return dijkstraMs_;
    }

private:
    std::vector<int> nodes_;
    std::map<int, int> index_;
    std::vector<std::vector<Step>> graph_;

    int source_ = -1;

    std::vector<double> distances_;
    std::vector<int> previous_;
    std::vector<int> previousLink_;

    mutable std::size_t dijkstraCalls_ = 0;
    mutable double dijkstraMs_ = 0;
};

struct RoutingProblem {
    int startNode;
    int endNode;

    double capacity;

    std::map<int, double> demand;
};

RoutingProblem makeProblem(
    const ArpInstance& instance,
    double capacity,
    double defaultDemand,
    const std::map<int, double>& overrides = {},
    int start = -1,
    int end = -1
);

RoutingProblem makeProblemFromInstance(
    const ArpInstance& instance,
    double capacity,
    const std::map<int, double>& overrides = {},
    int start = -1,
    int end = -1
);

struct Movement {
    int linkId;
    int from;
    int to;
    bool service;
};

struct Route {
    std::vector<TaskVisit> tasks;
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