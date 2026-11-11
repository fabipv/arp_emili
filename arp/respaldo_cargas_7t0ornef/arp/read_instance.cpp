#include "arp_instance.h"
#include <algorithm>   // std::min
#include <iomanip>     // std::setprecision
#include <iostream>    // std::cout y std::cerr
#include <stdexcept>   // Manejo de excepciones

int main(int argc, char** argv) {

    // Verifica que se haya proporcionado exactamente
    // un argumento: la ruta del archivo de instancia.
    // argc incluye también el nombre del ejecutable.
    if (argc != 2) {
        std::cerr << "Uso: arp_read RUTA_INSTANCIA\n";
        return 2;
    }

    try {

        // Crea un objeto que almacenará la información
        // de la instancia ARP.
        arp::ArpInstance instance;

        // Lee el archivo indicado por el usuario.
        // El parser interpreta los encabezados, las
        // calles requeridas y no requeridas, los nodos
        // iniciales y terminales, y valida los datos.
        instance.readDataFromFile(argv[1]);

        // Construye la lista de adyacencia del grafo.
        // Los enlaces 'uni' generan un solo movimiento
        // permitido, mientras que los 'bi' generan dos.
        const auto graph = instance.adjacency();

        // Contadores para almacenar:
        // directed: cantidad de calles unidireccionales.
        // steps: cantidad total de movimientos dirigidos
        //        presentes en la lista de adyacencia.
        std::size_t directed = 0, steps = 0;

        // Recorre todos los enlaces de la instancia.
        // Como directed es booleano, suma 1 cuando
        // el enlace es unidireccional y 0 si no lo es.
        for (const auto& link : instance.links)
            directed += link.directed;

        // Cuenta las conexiones almacenadas en el grafo.
        // Una calle 'uni' aporta un movimiento y una
        // calle 'bi' aporta dos movimientos.
        for (const auto& row : graph)
            steps += row.size();

        // Muestra la información general de la instancia.
        std::cout
            << "Nombre: " << instance.name

            // Comentario o descripción de la instancia.
            << "\nComentario: " << instance.comment

            // Cantidad de vértices indicada en el archivo.
            << "\nVertices declarados: "
            << instance.declaredVertices

            // Cantidad de nodos distintos identificados
            // al procesar los enlaces y nodos de inicio
            // y término.
            << "\nNodos identificables: "
            << instance.nodeIds.size()

            // Cantidad de enlaces que requieren servicio.
            << "\nCalles requeridas: "
            << instance.requiredLinkIds().size()

            // Cantidad de enlaces que no requieren
            // servicio, pero pueden utilizarse para
            // desplazarse por la red (dead-heading).
            << "\nCalles no requeridas: "
            << instance.links.size()
                - instance.requiredLinkIds().size()

            // Cantidad de enlaces unidireccionales.
            << "\nCalles uni: " << directed

            // Cantidad de enlaces bidireccionales.
            << "\nCalles bi: "
            << instance.links.size() - directed

            // Cantidad de movimientos dirigidos que
            // contiene la representación del grafo.
            << "\nPasos dirigidos en el grafo: "
            << steps

            << "\nNodos iniciales:";

        // Imprime todos los nodos desde los cuales
        // puede comenzar el recorrido.
        for (int node : instance.startNodes)
            std::cout << ' ' << node;

        std::cout << "\nNodos de termino:";

        // Imprime los nodos definidos como término.
        for (int node : instance.endNodes)
            std::cout << ' ' << node;

        // Si no hay nodos terminales definidos,
        // informa explícitamente esta situación.
        if (instance.endNodes.empty())
            std::cout << " no definidos";

        // Muestra la cantidad de restricciones
        // adicionales declaradas en el archivo.
        std::cout
            << "\nRestricciones declaradas: "
            << instance.declaredRestrictions << '\n';

        // Imprime los avisos generados por el parser.
        // Por ejemplo, diferencias entre los vértices
        // declarados y los nodos identificables, o
        // restricciones cuya semántica aún no se valida.
        for (const auto& warning : instance.warnings)
            std::cout << "AVISO: " << warning << '\n';

        // Configura la precisión de salida para mostrar
        // los costos sin redondearlos innecesariamente.
        std::cout << std::setprecision(12)
                  << "Primeras calles:\n";

        // Muestra como máximo los primeros cinco enlaces.
        // std::min evita acceder a posiciones inexistentes
        // cuando la instancia contiene menos de cinco.
        for (std::size_t i = 0;
             i < std::min<std::size_t>(
                 5, instance.links.size());
             ++i) {

            // Obtiene el enlace ubicado en la posición i.
            const auto& e = instance.links[i];

            // Imprime:
            // - Identificador del enlace.
            // - Tipo de enlace (uni o bi).
            // - Nodo origen y destino.
            // - Costo del enlace.
            // - Indicador de requerimiento (1 o 0).
            // - Línea original donde fue leído.
            std::cout
                << e.id << ": "
                << (e.directed ? "uni " : "bi ")
                << e.from << ' '
                << e.to << ' '
                << e.cost << ' '
                << e.indicator
                << " (linea " << e.sourceLine << ")\n";
        }

        // Finaliza correctamente la ejecución.
        return 0;

    } catch (const std::exception& e) {

        // Captura errores producidos durante la lectura,
        // validación o procesamiento de la instancia.
        // Muestra el mensaje para facilitar la depuración.
        std::cerr << "ERROR: " << e.what() << '\n';

        // Retorna un código distinto de cero indicando
        // que el programa terminó con un error.
        return 1;
    }
}
