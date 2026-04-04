from abc import ABC, abstractmethod
from typing import List, Optional, Any


class IObserver(ABC):
    @abstractmethod
    def update(self):
        pass


class Observable:
    def __init__(self):
        self._observers: List[IObserver] = []

    def attach(self, observer: IObserver):
        self._observers.append(observer)

    def detach(self, observer: IObserver):
        self._observers.remove(observer)

    def notify(self):
        for observer in self._observers:
            observer.update()


class Model(Observable, ABC):
    def __init__(self):
        super().__init__()

    @abstractmethod
    def get_state(self):
        pass

    @abstractmethod
    def restore_state(self, state):
        pass


class View(IObserver, ABC):
    def __init__(self, model: Model, adapter: 'IViewAdapter'):
        self.model = model
        model.attach(self)
        self.adapter = adapter

    @abstractmethod
    def update(self):
        pass


class Controller(ABC):
    def __init__(self, adapter: 'IControllerAdapter', model: Model):
        self.model = model
        self.running = True
        self.adapter = adapter

    @abstractmethod
    def run(self):
        pass

    @abstractmethod
    def stop(self):
        pass

class MVCApplication:
    def __init__(self):
        self.model: Optional[Model] = None
        self.view: Optional[View] = None
        self.controller: Optional[Controller] = None
        self.adapter: Optional[Any] = None
        self.name: str = "Unknown"


    def run(self):
        if self.controller:
            self.controller.run()

    def stop(self):
        if self.controller:
            self.controller.stop()

    def __str__(self):
        return f"MVCApplication(name='{self.name}', " \
               f"model={type(self.model).__name__}, " \
               f"view={type(self.view).__name__}, " \
               f"controller={type(self.controller).__name__})"