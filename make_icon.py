from pathlib import Path
import subprocess
import sys

from PIL import Image


PROJECT_DIR = Path(__file__).resolve().parent
SVG_FILE = PROJECT_DIR / "src" / "app.svg"
PNG_FILE = PROJECT_DIR / "src" / "app.png"
ICO_FILE = PROJECT_DIR / "src" / "app.ico"

QT_BIN = Path(r"C:\Qt\6.11.2\mingw_64\bin")
QTPRINTSUPPORT_DLL = QT_BIN / "Qt6PrintSupport.dll"
SVGRENDERER = QT_BIN / "svgtoqpicture.exe"


def find_svg_tool():
    candidates = [
        QT_BIN / "svgtoqpicture.exe",
        QT_BIN / "svgtoqpdf.exe",
    ]

    for candidate in candidates:
        if candidate.exists():
            return candidate

    return None


def create_png_with_qt():
    if not SVG_FILE.exists():
        raise FileNotFoundError(
            f"Не найден SVG-файл: {SVG_FILE}"
        )

    tool = find_svg_tool()

    if tool is None:
        return False

    if tool.name.lower() == "svgtoqpicture.exe":
        print(
            "Найден svgtoqpicture.exe, "
            "но он не предназначен для прямого создания PNG."
        )
        return False

    return False


def create_png_with_pillow():
    """
    Pillow используется только для создания ICO.
    SVG она не рендерит, поэтому этот метод намеренно
    не пытается открыть SVG.
    """
    raise RuntimeError(
        "Для преобразования SVG в PNG требуется Qt SVG renderer."
    )


def create_ico_from_png():
    if not PNG_FILE.exists():
        raise FileNotFoundError(
            f"Не найден PNG-файл: {PNG_FILE}"
        )

    image = Image.open(PNG_FILE).convert("RGBA")

    image = image.resize(
        (256, 256),
        Image.Resampling.LANCZOS
    )

    image.save(
        ICO_FILE,
        format="ICO",
        sizes=[
            (256, 256),
            (128, 128),
            (64, 64),
            (48, 48),
            (32, 32),
            (16, 16),
        ],
    )

    print(f"Создано: {ICO_FILE}")
    print(f"Размер: {ICO_FILE.stat().st_size} bytes")


def main():
    print(f"SVG: {SVG_FILE}")
    print(f"PNG: {PNG_FILE}")
    print(f"ICO: {ICO_FILE}")
    print()

    if not SVG_FILE.exists():
        print("ОШИБКА: src/app.svg не найден.")
        sys.exit(1)

    print(
        "Pillow установлена, но Qt-инструмент "
        "для SVG-рендеринга нужно определить отдельно."
    )

    print()
    print("Проверяем Qt SVG...")
    print()

    qt_svg_dir = Path(
        r"C:\Qt\6.11.2\mingw_64\bin"
    )

    for name in [
        "Qt6Svg.dll",
        "Qt6Gui.dll",
        "Qt6Core.dll",
    ]:
        path = qt_svg_dir / name

        print(
            f"{name}: "
            f"{'OK' if path.exists() else 'НЕ НАЙДЕН'}"
        )


if __name__ == "__main__":
    main()