from mvc_framework.base import Model


class CounterModel(Model):
    def __init__(self):
        super().__init__()
        self.value = 0
        self.max_value = 100
        self.min_value = 0

    def increment(self):
        if self.value < self.max_value:
            self.value += 1
            self.notify()

    def decrement(self):
        if self.value > self.min_value:
            self.value -= 1
            self.notify()

    def reset(self):
        self.value = 0
        self.notify()

    def get_state(self):
        return {'value': self.value}

    def restore_state(self, state):
        self.value = state['value']
        self.notify()