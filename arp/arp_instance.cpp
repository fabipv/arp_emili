#include "arp_instance.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>

// Agrupa las clases y funciones relacionadas con Arc Routing Problems.
namespace arp {
// Namespace anonimo: estas utilidades solo son visibles en este archivo.
namespace {

// Elimina espacios, tabulaciones y saltos de linea al inicio y al final.
std::string trim(const std::string& text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    return text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);
}

// Centraliza los errores de lectura e indica la linea donde ocurrieron.
// [[noreturn]] expresa que siempre lanza una excepcion.
[[noreturn]] void fail(int line, const std::string& message) {
    throw std::runtime_error("linea " + std::to_string(line) + ": " + message);
}

// Lee enteros no negativos y verifica que quepan en un int.
// Rechaza signos, caracteres extra y valores demasiado grandes.
int parseInteger(const std::string& text, int line) {
    if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos)
        fail(line, "se esperaba un entero no negativo: " + text);
    try {
        const long long value = std::stoll(text);
        if (value > std::numeric_limits<int>::max()) fail(line, "entero fuera de rango");
        return static_cast<int>(value);
    } catch (const std::out_of_range&) {
        fail(line, "entero fuera de rango");
    }
}

// Lee costos reales: deben ser finitos y mayores o iguales a cero.
// Se comprueba que toda la cadena haya sido consumida por std::stod.
double parseCost(const std::string& text, int line) {
    try {
        std::size_t used = 0;
        const double value = std::stod(text, &used);
        if (used != text.size() || !std::isfinite(value) || value < 0)
            fail(line, "se esperaba un costo no negativo y finito: " + text);
        return value;
    } catch (const std::invalid_argument&) {
        fail(line, "costo invalido: " + text);
    } catch (const std::out_of_range&) {
        fail(line, "costo fuera de rango");
    }
}

// Convierte una fila de identificadores de nodos en una lista de enteros.
void readNodeRow(const std::string& row, std::vector<int>& nodes, int line) {
    std::istringstream input(row);
    std::string token;
    while (input >> token) nodes.push_back(parseInteger(token, line));
}

// Analiza el archivo completo y retorna una instancia validada.
// Las excepciones describen problemas de formato o contenido.
ArpInstance parse(const std::string& path) {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("no se puede abrir el archivo: " + path);

    ArpInstance instance;
    // Cabeceras: NOMBRE, VERTICES, cantidades declaradas, etc.
    std::map<std::string, std::string> headers;
    // Linea de cada cabecera y numero de enlaces efectivamente leidos.
    std::map<std::string, int> headerLines, linkCounts;
    // Permite detectar secciones duplicadas en el archivo.
    std::set<std::string> sections;
    // Claves que se aceptan como metadatos.
    const std::set<std::string> allowedHeaders = {
        "NOMBRE", "COMENTARIO", "VERTICES", "ARISTAS_REQ", "ARISTAS_NOREQ",
        "RESTRICCIONES", "NODOS_INICIALES", "NODOS_TERMINO"
    };
    // Secciones que contienen filas de datos. Se aceptan ambas grafias
    // de COORDENADAS y CORDENADAS para compatibilidad.
    const std::set<std::string> allowedSections = {
        "LISTA_ARISTAS_REQ", "LISTA_ARISTAS_NOREQ", "CORDENADAS_NODOS",
        "COORDENADAS_NODOS", "RESTRICCIONES", "NODOS_INICIALES", "NODOS_TERMINO"
    };
    std::string line, section;
    int lineNumber = 0;
    bool ended = false; // EOF impide aceptar contenido posterior.

    // Recorre el archivo linea por linea, preservando el numero de linea.
    while (std::getline(input, line)) {
        ++lineNumber;
        // Elimina la marca UTF-8 BOM si esta presente al principio del archivo.
        if (lineNumber == 1 && line.compare(0, 3, "\xef\xbb\xbf") == 0) line.erase(0, 3);
        line = trim(line);
        if (line.empty()) continue;
        if (ended) fail(lineNumber, "contenido despues de EOF");
        if (line == "EOF") { ended = true; continue; }

        // Las cabeceras y los encabezados de seccion contienen dos puntos.
        const auto colon = line.find(':');
        if (colon != std::string::npos) {
            const std::string key = trim(line.substr(0, colon));
            const std::string value = trim(line.substr(colon + 1));
            // Algunas filas de coordenadas/restricciones pueden contener ":".
            // Se guardan literalmente en vez de tratarlas como claves.
            if (!allowedHeaders.count(key) && !allowedSections.count(key)) {
                if (section == "CORDENADAS_NODOS" || section == "COORDENADAS_NODOS") {
                    instance.coordinateRows.push_back(line);
                    continue;
                }
                if (section == "RESTRICCIONES") {
                    instance.restrictionRows.push_back(line);
                    continue;
                }
            }
            // Un conteo tiene valor. una sección usa el mismo rótulo pero queda vacía.
            // Una seccion (ej. LISTA_ARISTAS_REQ :) no tiene valor
            // despues de los dos puntos; una cabecera si lo tiene.
            if (value.empty() && allowedSections.count(key)) {
                if (!sections.insert(key).second) fail(lineNumber, "seccion duplicada: " + key);
                if (key == "CORDENADAS_NODOS" || key == "COORDENADAS_NODOS") {
                    if (!instance.coordinatesLabel.empty()) fail(lineNumber, "seccion de coordenadas duplicada");
                    instance.coordinatesLabel = key;
                }
                section = key;
            } else {
                if (!allowedHeaders.count(key)) fail(lineNumber, "cabecera desconocida: " + key);
                if (!headers.emplace(key, value).second) fail(lineNumber, "cabecera duplicada: " + key);
                headerLines[key] = lineNumber;
                section.clear();
            }
            continue;
        }

        // Interpreta cada fila segun la seccion activa.
        if (section == "NODOS_INICIALES") {
            readNodeRow(line, instance.startNodes, lineNumber);
        } else if (section == "NODOS_TERMINO") {
            readNodeRow(line, instance.rawEndNodes, lineNumber);
        } else if (section == "CORDENADAS_NODOS" || section == "COORDENADAS_NODOS") {
            instance.coordinateRows.push_back(line);
        } else if (section == "RESTRICCIONES") {
            instance.restrictionRows.push_back(line);
        } else if (section == "LISTA_ARISTAS_REQ" || section == "LISTA_ARISTAS_NOREQ") {
            std::istringstream row(line);
            std::string type, from, to, cost, demand, extra;
            // Cada enlace debe contener exactamente cinco campos.
            if (!(row >> type >> from >> to >> cost >> demand) || (row >> extra))
                fail(lineNumber, "se esperaba: uni|bi origen destino tiempo carga");
            if (type != "uni" && type != "bi") fail(lineNumber, "sentido desconocido: " + type);
            const int load = parseInteger(demand, lineNumber);
            // La seccion determina si el enlace exige servicio.
            const bool required = section == "LISTA_ARISTAS_REQ";
            if (required && load == 0)
                fail(lineNumber, "una calle requerida debe tener carga positiva");
            if (!required && load != 0)
                fail(lineNumber, "una calle no requerida debe tener carga 0");
            if (instance.links.size() >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
                fail(lineNumber, "demasiados enlaces");
            // El ID interno comienza en 1; "uni" es dirigido y "bi" no.
            // Se conserva el costo original y la linea para trazabilidad.
            instance.links.push_back({static_cast<int>(instance.links.size()) + 1,
                parseInteger(from, lineNumber), parseInteger(to, lineNumber), type == "uni",
                parseCost(cost, lineNumber), required, load, lineNumber});
            ++linkCounts[section];
        } else {
            fail(lineNumber, "fila fuera de una seccion reconocida");
        }
    }
    if (input.bad()) throw std::runtime_error("error al leer el archivo: " + path);

    // Obtiene cabeceras obligatorias; lanza error si falta alguna.
    auto get = [&](const std::string& key) -> std::string {
        if (!headers.count(key)) fail(lineNumber, "falta cabecera " + key);
        return headers.at(key);
    };
    auto getInteger = [&](const std::string& key) -> int {
        const std::string value = get(key);
        return parseInteger(value, headerLines.at(key));
    };
    // Transfiere al objeto los metadatos ya leidos.
    instance.name = get("NOMBRE");
    instance.comment = get("COMENTARIO");
    instance.declaredVertices = getInteger("VERTICES");
    instance.declaredRequired = getInteger("ARISTAS_REQ");
    instance.declaredNonRequired = getInteger("ARISTAS_NOREQ");
    instance.declaredRestrictions = getInteger("RESTRICCIONES");
    instance.declaredStarts = getInteger("NODOS_INICIALES");
    instance.declaredEnds = getInteger("NODOS_TERMINO");
    // Valida coherencia entre cantidades declaradas y datos reales.
    if (instance.declaredVertices <= 0) fail(lineNumber, "VERTICES debe ser positivo");
    if (!sections.count("LISTA_ARISTAS_REQ") || !sections.count("LISTA_ARISTAS_NOREQ"))
        fail(lineNumber, "faltan las listas de calles requeridas o no requeridas");
    if (instance.declaredRequired != linkCounts["LISTA_ARISTAS_REQ"])
        fail(lineNumber, "ARISTAS_REQ no coincide con las filas leidas");
    if (instance.declaredNonRequired != linkCounts["LISTA_ARISTAS_NOREQ"])
        fail(lineNumber, "ARISTAS_NOREQ no coincide con las filas leidas");
    if (instance.declaredStarts != static_cast<int>(instance.startNodes.size()))
        fail(lineNumber, "NODOS_INICIALES no coincide con los nodos leidos");

    // Conserva los valores originales de termino y resuelve el caso
    // especial en que el archivo usa 0 como marcador de ausencia.
    instance.endNodes = instance.rawEndNodes;
    if (instance.declaredEnds == 0 && instance.rawEndNodes == std::vector<int>{0}) {
        instance.hasZeroEndMarker = true;
        instance.endNodes.clear();
        instance.warnings.push_back("NODOS_TERMINO declara 0: la fila final 0 se conserva como marcador, sin crear un nodo de termino.");
    }
    if (instance.declaredEnds != static_cast<int>(instance.endNodes.size()))
        fail(lineNumber, "NODOS_TERMINO no coincide con los nodos leidos");

    // Reune identificadores reales, que no tienen por que ser 0..N-1.
    std::set<int> observed;
    for (const auto& link : instance.links) { observed.insert(link.from); observed.insert(link.to); }
    for (int node : instance.startNodes) observed.insert(node);
    for (int node : instance.endNodes) observed.insert(node);
    instance.nodeIds.assign(observed.begin(), observed.end());
    // Si el numero declarado difiere del detectado, solo advierte:
    // no inventa nodos ni elimina informacion del archivo.
    if (instance.declaredVertices != static_cast<int>(instance.nodeIds.size()))
        instance.warnings.push_back("VERTICES declarado=" + std::to_string(instance.declaredVertices)
            + ", nodos identificables=" + std::to_string(instance.nodeIds.size())
            + ". Se conservan ambos datos sin modificar el archivo.");
    if (instance.startNodes != instance.endNodes)
        instance.warnings.push_back("Inicio y termino diferentes o ausentes: se conservan sin imponer viajes cerrados.");
    if (instance.declaredRestrictions != 0 || !instance.restrictionRows.empty())
        instance.warnings.push_back("Las restricciones adicionales se conservan como texto; su semantica y cumplimiento aun no se validan.");
    if (!instance.coordinateRows.empty())
        instance.warnings.push_back("Las coordenadas se conservan como texto, sin inferir su formato ni cambiar los costos.");
    // Entrega la instancia con enlaces, nodos y advertencias.
    return instance;
}

} // namespace

// Carga una instancia en el objeto actual.
// Primero parsea por completo y luego reemplaza el estado: si falla,
// el objeto existente no se modifica.
bool ArpInstance::readDataFromFile(const std::string& path) {
    ArpInstance loaded = parse(path);
    *this = std::move(loaded);
    return true;
}

// Construye una lista de adyacencia con indices internos consecutivos.
// Para "uni" crea solo from -> to; para "bi" crea ambos sentidos.
// Una arista "bi" sigue siendo un unico enlace/tarea con el mismo ID.
std::vector<std::vector<Step>> ArpInstance::adjacency() const {
    std::map<int, int> index;
    for (std::size_t i = 0; i < nodeIds.size(); ++i) index[nodeIds[i]] = static_cast<int>(i);
    std::vector<std::vector<Step>> graph(nodeIds.size());
    for (const auto& link : links) {
        const int u = index.at(link.from), v = index.at(link.to);
        graph[u].push_back({v, link.id, link.cost});
        if (!link.directed) graph[v].push_back({u, link.id, link.cost});
    }
    return graph;
}

// Devuelve los ID de enlaces que obligatoriamente deben servirse.
// Los no requeridos no aparecen aqui, pero siguen en el grafo para transito.
std::vector<int> ArpInstance::requiredLinkIds() const {
    std::vector<int> ids;
    for (const auto& link : links) if (link.required) ids.push_back(link.id);
    return ids;
}

} // namespace arp