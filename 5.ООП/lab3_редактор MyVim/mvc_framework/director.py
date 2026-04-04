from mvc_framework.builder import MVCBuilder


class MVCDirector:
    def __init__(self, builder: MVCBuilder = None):
        self._builder = builder

    @property
    def builder(self) -> MVCBuilder:
        return self._builder

    @builder.setter
    def builder(self, builder: MVCBuilder):
        self._builder = builder

    def build_minimal_app(self, adapter, name: str = "App"):
        if not self._builder:
            raise ValueError("Builder not set")
        return self._builder.reset() \
            .set_name(name) \
            .set_adapter(adapter) \
            .build_model() \
            .build_view() \
            .build_controller() \
            .get_application()

    def build_full_app(self, adapter, name: str = "App"):
        if not self._builder:
            raise ValueError("Builder not set")
        # дополнительная логика для более маштабного приложения
        return self.build_minimal_app(adapter, name)