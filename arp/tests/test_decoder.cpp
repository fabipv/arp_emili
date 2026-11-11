#include "arp_decoder.h"

#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {

void check(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

arp::ArpInstance line() {
    arp::ArpInstance i;

    i.nodeIds = {0, 1, 2};
    i.startNodes = {0};
    i.endNodes = {0};

    // id, from, to, directed, cost, required, demand, sourceLine
    i.links = {
        {10, 0, 1, false, 1, true, 7, 1},
        {20, 1, 2, false, 1, true, 4, 2}
    };

    return i;
}

void testProfessorExample() {
    const auto i = line();
    const auto p = arp::makeProblemFromInstance(i, 8);

    arp::ShortestPaths paths(i);

    const std::vector<arp::TaskVisit> priority{
        {10, false},
        {20, false}
    };

    // Verifica tambien que la expulsion del cache
    // no invalide los caminos del candidato actual.
    for (std::size_t cacheSize : {0u, 1u, 16u}) {
        const auto s = arp::decodeFirstTraversal(
            i,
            p,
            priority,
            paths,
            cacheSize
        );

        check(
            s.complete && s.routes.size() == 2,
            "Ejemplo: dos rutas"
        );

        check(
            s.routes[0].load == 7
                && s.routes[1].load == 4,
            "Ejemplo: cargas 7 y 4"
        );

        check(s.cost == 6, "Ejemplo: tiempo total 6");

        check(
            arp::validateFirstTraversal(i, p, s).valid(),
            "Ejemplo: validacion"
        );

        auto bad = s;
        bad.routes[0].walk[0].service = false;

        check(
            !arp::validateFirstTraversal(i, p, bad).valid(),
            "Detecta primera recogida omitida"
        );

        bad = s;
        bad.routes[1].walk[0].service = true;

        check(
            !arp::validateFirstTraversal(i, p, bad).valid(),
            "Detecta servicio duplicado global"
        );

        bad = s;
        bad.routes[0].load += 1;

        check(
            !arp::validateFirstTraversal(i, p, bad).valid(),
            "Detecta carga incorrecta"
        );

        bad = s;
        bad.cost += 1;

        check(
            !arp::validateFirstTraversal(i, p, bad).valid(),
            "Detecta tiempo incorrecto"
        );
    }

    const auto blocked = arp::decodeFirstTraversal(
        i,
        p,
        {{20, false}, {10, false}},
        paths
    );

    check(
        !blocked.complete && blocked.routes.empty(),
        "Prioridad bloqueada sin progreso"
    );

    check(
        !arp::validateFirstTraversal(i, p, blocked).valid(),
        "Fallo no es solucion valida"
    );
}

void testEarlyCollection() {
    arp::ArpInstance i;

    i.nodeIds = {0, 1, 2, 3, 4};
    i.startNodes = {0};
    i.endNodes = {0};

    i.links = {
        {10, 0, 1, false, 1, true, 2, 1},
        {20, 1, 2, false, 1, true, 2, 2},
        {30, 2, 3, false, 1, true, 2, 3},
        {40, 3, 4, false, 1, false, 0, 4},
        {50, 4, 0, false, 2, false, 0, 5}
    };

    const auto p = arp::makeProblemFromInstance(i, 4);
    arp::ShortestPaths paths(i);

    const auto s = arp::decodeFirstTraversal(
        i,
        p,
        {{10, false}, {30, false}, {20, true}},
        paths
    );

    check(
        s.complete && s.cost == 8 && s.routes.size() == 2,
        "Recoleccion anticipada"
    );

    check(
        s.routes[1].tasks.size() == 2
            && s.routes[1].tasks[0].taskId == 20,
        "Orden real diferente de prioridad"
    );

    check(
        arp::validateFirstTraversal(i, p, s).valid(),
        "Valida orden real"
    );

    auto bad = s;
    bad.routes[0].walk[0].from = 4;

    check(
        !arp::validateFirstTraversal(i, p, bad).valid(),
        "Detecta discontinuidad"
    );
}

void testReturnAndCapacity() {
    arp::ArpInstance i;

    i.nodeIds = {0, 1, 2};
    i.startNodes = {0};
    i.endNodes = {0};

    i.links = {
        {10, 0, 1, false, 10, true, 2, 1},
        {20, 1, 2, false, 1, true, 3, 2},
        {30, 2, 0, false, 1, false, 0, 3}
    };

    arp::ShortestPaths paths(i);

    auto p = arp::makeProblemFromInstance(i, 5);

    const auto s = arp::decodeFirstTraversal(
        i,
        p,
        {{10, false}, {20, false}},
        paths
    );

    check(
        s.complete
            && s.routes.size() == 1
            && s.routes[0].load == 5,
        "Retorno recolecta y permite carga igual a Q"
    );

    check(
        s.routes[0].tasks[1].taskId == 20
            && s.cost == 12,
        "Servicio durante retorno"
    );

    check(
        arp::validateFirstTraversal(i, p, s).valid(),
        "Validacion del retorno"
    );

    p = arp::makeProblemFromInstance(i, 4);

    const auto blocked = arp::decodeFirstTraversal(
        i,
        p,
        {{10, false}, {20, false}},
        paths
    );

    check(
        !blocked.complete,
        "Rechaza retorno que excede capacidad"
    );
}

void testDirectedAndEmpty() {
    arp::ArpInstance i;

    i.nodeIds = {0, 1};
    i.startNodes = {0};
    i.endNodes = {0};

    i.links = {
        {10, 0, 1, true, 1, true, 1, 1},
        {20, 1, 0, true, 1, false, 0, 2}
    };

    arp::ShortestPaths paths(i);

    const auto p = arp::makeProblemFromInstance(i, 1);

    auto s = arp::decodeFirstTraversal(
        i,
        p,
        {{10, false}},
        paths
    );

    check(
        s.complete
            && arp::validateFirstTraversal(i, p, s).valid(),
        "Calles uni"
    );

    s.routes[0].walk[0].from = 1;
    s.routes[0].walk[0].to = 0;

    check(
        !arp::validateFirstTraversal(i, p, s).valid(),
        "Detecta inversion de uni"
    );

    auto j = line();

    const auto decimal = arp::makeProblem(
        j,
        0.3,
        0,
        {{10, 0.1}, {20, 0.2}}
    );

    arp::ShortestPaths decimalPaths(j);

    const auto d = arp::decodeFirstTraversal(
        j,
        decimal,
        {{10, false}, {20, false}},
        decimalPaths
    );

    check(
        d.complete
            && d.routes.size() == 1
            && arp::validateFirstTraversal(
                j,
                decimal,
                d
            ).valid(),
        "Tolerancia decimal comun"
    );

    arp::ArpInstance empty;

    empty.nodeIds = {0};
    empty.startNodes = {0};
    empty.endNodes = {0};

    arp::ShortestPaths emptyPaths(empty);

    const auto emptyProblem =
        arp::makeProblemFromInstance(empty, 1);

    const auto e = arp::decodeFirstTraversal(
        empty,
        emptyProblem,
        {},
        emptyPaths
    );

    check(
        e.complete
            && e.routes.empty()
            && e.cost == 0
            && arp::validateFirstTraversal(
                empty,
                emptyProblem,
                e
            ).valid(),
        "Sin servicios"
    );
}

} // namespace

int main() {
    try {
        testProfessorExample();
        testEarlyCollection();
        testReturnAndCapacity();
        testDirectedAndEmpty();

        std::cout
            << "OK: primera travesia, retorno, capacidad, "
            << "cache y validacion\n";

        return 0;

    } catch (const std::exception& error) {
        std::cerr
            << "ERROR: "
            << error.what()
            << '\n';

        return 1;
    }
}