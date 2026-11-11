#ifndef EMILI_ARP_INSTANCE_H
#define EMILI_ARP_INSTANCE_H

#include <string>
#include <vector>

namespace arp {

// Una fila de LISTA_ARISTAS_REQ o LISTA_ARISTAS_NOREQ.
struct Link {
    int id;                 // ID interno de enlace, empezando en 1
    int from, to;           // Identificadores originales de los nodos
    bool directed;          // true = uni; false = bi
    double cost;            // Valor original, sin convertir unidades
    bool required;          // Sección a la que pertenece la fila
    int indicator;          // Última columna original (0/1)
    int sourceLine;
};

// Movimiento posible dentro del grafo
struct Step {
    int toIndex;            // Índice interno en nodeIds; no es el ID original.
    int linkId;
    double cost;
};

class ArpInstance {
public:
    std::string name, comment;
    int declaredVertices = 0;
    int declaredRequired = 0, declaredNonRequired = 0;
    int declaredRestrictions = 0;
    int declaredStarts = 0, declaredEnds = 0;

    std::vector<Link> links;                // Una entrada por fila original.
    std::vector<int> nodeIds;               // IDs identificables, ordenados y sin repetición.
    std::vector<int> startNodes, endNodes;   // Valores utilizables según los conteos.
    std::vector<int> rawEndNodes;           // Conserva también el marcador final 0.
    bool hasZeroEndMarker = false;

    // Secciones conservadas sin atribuirles una semántica que el archivo no especifica.
    std::string coordinatesLabel;
    std::vector<std::string> coordinateRows, restrictionRows;
    std::vector<std::string> warnings;

    // Devuelve true al completar la lectura. Ante error lanza std::runtime_error.
    // Si falla, el objeto conserva su contenido anterior.
    bool readDataFromFile(const std::string& path);
    std::vector<std::vector<Step>> adjacency() const;
    std::vector<int> requiredLinkIds() const;
};

} // namespace arp
#endif
