from mvc_framework.base import View


class CounterView(View):

    def update(self):
        self.adapter.clear()
        h, w = self.adapter.get_window_size()

        title = " COUNTER DEMO "
        value_str = f" Value: {self.model.value} "
        instructions = " Up/Down: Change | R: Reset | Q: Quit "

        self.adapter.addstr(h // 2 - 1, (w - len(title)) // 2, title)
        self.adapter.addstr(h // 2, (w - len(value_str)) // 2, value_str)
        self.adapter.addstr(h // 2 + 1, (w - len(instructions)) // 2, instructions)

        self.adapter.refresh()