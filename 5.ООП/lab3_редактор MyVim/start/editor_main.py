import curses
import sys

from start.editor_builder import TextEditorBuilder

from editor.view.curses_adapter import CursesAdapter
from mvc_framework.director import MVCDirector


def main(stdscr):
    adapter = CursesAdapter(stdscr)
    builder = TextEditorBuilder()
    director = MVCDirector(builder)
    filename = sys.argv[1] if len(sys.argv) > 1 else None
    if filename:
        app = director.build_minimal_app(adapter, f"Editor - {filename}")
        app.model.filename = filename
    else:
        app = director.build_minimal_app(adapter, "Text Editor")
    #print(f"Created application: {app}")
    app.run()


if __name__ == "__main__":
    curses.wrapper(main)