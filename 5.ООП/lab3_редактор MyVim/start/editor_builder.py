from mvc_framework.builder import MVCBuilder


class TextEditorBuilder(MVCBuilder):
    def build_model(self):
        from editor.model.text_model import EditorModel
        self.application.model = EditorModel()
        return self

    def build_view(self):
        from editor.view.view_editor import EditorView
        if not self.application.model:
            raise ValueError("Model must be built before view")
        if not self.application.adapter:
            raise ValueError("Adapter must be set before building view")
        self.application.view = EditorView(self.application.model, self.application.adapter)
        return self

    def build_controller(self):
        from editor.controller.controller_editor import EditorController
        if not self.application.model:
            raise ValueError("Model must be built before controller")
        if not self.application.adapter:
            raise ValueError("Adapter must be set before building view")
        self.application.controller = EditorController(
            self.application.adapter,
            self.application.model
        )
        return self