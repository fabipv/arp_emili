#!/usr/bin/env python3
"""Quinta columna: cargas enteras 1..20 en requeridas y 0 en no requeridas."""
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import random
import re
import shutil
import tempfile

ROW = re.compile(
    r"(?P<prefix>[ \t]*(?:uni|bi)[ \t]+[0-9]+[ \t]+[0-9]+[ \t]+\S+[ \t]+)"
    r"(?P<load>[0-9]+)(?P<suffix>[ \t\r\n]*)"
)


def convert(data, relative, seed):
    # Una secuencia propia por archivo: agregar otra instancia no altera las demas.
    digest = hashlib.sha256((str(seed) + "\0" + relative).encode("utf-8")).digest()
    rng = random.Random(int.from_bytes(digest, "big"))
    section = ""
    headers = {}
    counts = {"LISTA_ARISTAS_REQ": 0, "LISTA_ARISTAS_NOREQ": 0}
    distribution = {value: 0 for value in range(1, 21)}
    output = []
    changed = 0
    for line_number, line in enumerate(data.decode("utf-8").splitlines(keepends=True), 1):
        stripped = line.strip()
        if ":" in stripped:
            key, value = (part.strip() for part in stripped.split(":", 1))
            if key in ("ARISTAS_REQ", "ARISTAS_NOREQ"):
                headers[key] = int(value)
            section = key if not value else ""
        elif section in counts and stripped and stripped != "EOF":
            match = ROW.fullmatch(line)
            if not match:
                raise ValueError(f"{relative}, linea {line_number}: fila de calle invalida")
            time = float(match.group("prefix").split()[-1])
            if not math.isfinite(time) or time < 0:
                raise ValueError(f"{relative}, linea {line_number}: tiempo invalido")
            load = rng.randint(1, 20) if section == "LISTA_ARISTAS_REQ" else 0
            if load:
                distribution[load] += 1
            counts[section] += 1
            changed += match.group("load") != str(load)
            line = match.group("prefix") + str(load) + match.group("suffix")
        output.append(line)
    for key, section_name in (("ARISTAS_REQ", "LISTA_ARISTAS_REQ"),
                              ("ARISTAS_NOREQ", "LISTA_ARISTAS_NOREQ")):
        if headers.get(key) != counts[section_name]:
            raise ValueError(f"{relative}: {key} no coincide con sus filas")
    converted = "".join(output).encode("utf-8")
    return converted, {
        "archivo": relative,
        "requeridas": counts["LISTA_ARISTAS_REQ"],
        "no_requeridas": counts["LISTA_ARISTAS_NOREQ"],
        "filas_modificadas": changed,
        "distribucion_cargas": distribution,
        "sha256_original": hashlib.sha256(data).hexdigest(),
        "sha256_generado": hashlib.sha256(converted).hexdigest(),
    }


def atomic_write(target, content, source):
    target.parent.mkdir(parents=True, exist_ok=True)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(dir=target.parent, delete=False) as stream:
            temporary = Path(stream.name)
            stream.write(content)
            stream.flush()
            os.fsync(stream.fileno())
        os.chmod(temporary, source.stat().st_mode)
        os.replace(temporary, target)
    finally:
        if temporary is not None and temporary.exists():
            temporary.unlink()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", type=Path, help="Carpeta Instancias_redes_viales")
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--output", type=Path, help="Generar una copia en otra carpeta")
    mode.add_argument("--in-place", action="store_true", help="Modificar con respaldo previo")
    parser.add_argument("--seed", type=int, default=42)
    args = parser.parse_args()
    root = args.root.resolve()
    if not root.is_dir():
        parser.error("La carpeta de instancias no existe")
    destination = root if args.in_place else args.output.resolve()
    if not args.in_place and (destination == root or root in destination.parents):
        parser.error("--output debe estar fuera de la carpeta de entrada")
    files = sorted(p for p in root.rglob("*") if p.is_file()
                   and (p.suffix.lower() == ".txt" or p.name == "Renca_Entero"))
    if not files:
        parser.error("No se encontraron instancias")
    # Analiza todos los archivos antes de escribir alguno.
    prepared = []
    for source in files:
        relative = source.relative_to(root).as_posix()
        converted, report = convert(source.read_bytes(), relative, args.seed)
        prepared.append((source, converted, report))
    backup = None
    if args.in_place:
        backup = Path(tempfile.mkdtemp(prefix=root.name + "_respaldo_", dir=root.parent))
        for source in files:
            target = backup / source.relative_to(root)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, target)
    for source, converted, report in prepared:
        target = destination / source.relative_to(root)
        atomic_write(target, converted, source)
        print(f"OK: {report['archivo']} | requeridas={report['requeridas']} "
              f"| no requeridas={report['no_requeridas']}")
    manifest = {
        "semilla": args.seed,
        "regla": "carga requerida: entero aleatorio uniforme 1..20; no requerida: 0",
        "respaldo": str(backup) if backup else None,
        "instancias": [report for _, _, report in prepared],
    }
    (destination / "cargas_manifest.json").write_text(
        json.dumps(manifest, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"Instancias procesadas: {len(files)} | semilla: {args.seed}")
    if backup:
        print(f"Respaldo: {backup}")


if __name__ == "__main__":
    main()
