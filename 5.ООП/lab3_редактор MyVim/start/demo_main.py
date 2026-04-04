import curses
from start.counter_builder import CounterBuilder
from mvc_framework.director import MVCDirector
from editor.view.curses_adapter import CursesAdapter


def main(stdscr):
    adapter = CursesAdapter(stdscr)
    builder = CounterBuilder()
    director = MVCDirector(builder)
    app = director.build_minimal_app(adapter, "Counter")
    #print(f"Created application: {app}")
    app.run()


if __name__ == "__main__":
    curses.wrapper(main)