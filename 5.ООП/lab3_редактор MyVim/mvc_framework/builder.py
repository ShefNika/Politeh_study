from abc import ABC, abstractmethod
from typing import Any

from mvc_framework.base import MVCApplication



class MVCBuilder(ABC):
    def __init__(self):
        self.application = MVCApplication()

    def reset(self):
        # cбросить для создания нового приложения
        self.application = MVCApplication()
        return self

    def set_name(self, name: str):
        self.application.name = name
        return self

    def set_adapter(self, adapter: Any):
        self.application.adapter = adapter
        return self

    @abstractmethod
    def build_model(self):
        pass

    @abstractmethod
    def build_view(self):
        pass

    @abstractmethod
    def build_controller(self):
        pass

    def get_application(self) -> MVCApplication:
        app = self.application
        self.reset()
        return app
