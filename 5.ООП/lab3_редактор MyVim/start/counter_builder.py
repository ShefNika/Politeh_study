from mvc_framework.builder import MVCBuilder
from demo.counter_model import CounterModel
from demo.counter_view import CounterView
from demo.counter_controller import CounterController


class CounterBuilder(MVCBuilder):
    def build_model(self):
        self.application.model = CounterModel()
        return self

    def build_view(self):
        if not self.application.model:
            raise ValueError("Model must be built before view")
        if not self.application.adapter:
            raise ValueError("Adapter must be set before building view")
        self.application.view = CounterView(self.application.model, self.application.adapter)
        return self

    def build_controller(self):
        if not self.application.model:
            raise ValueError("Model must be built before controller")
        if not self.application.adapter:
            raise ValueError("Adapter must be set before building view")
        self.application.controller = CounterController(self.application.adapter, self.application.model)
        return self