#ifndef AHO_CORASICK
#define AHO_CORASICK

#include <queue>  
#include <cstring>  


class Aho {
    // Структура для узла в боре (Vertex — вершина)
    struct Vertex {
        int children[256];  // Массив детей: индекс = (int)c, значение = индекс узла (-1 если нет). Для ASCII (0-255).
        int parent = -1;  // Родительский узел
        int suffix_link = -1;  // Суффиксная ссылка
        int end_word_link = -1;  // Ссылка на конец слова
        int word_ID = -1;  // ID слова, если это лист (не обязательно для одного слова)
        char parent_char;  // Буква от родителя к этому узлу
        bool leaf = false;  // Флаг: конец слова?
        int word_length = 0;  // Длина слова (для этого листа)
    };

    Vertex* trie;  // Динамический массив узлов
    int size;  // Количество узлов (размер массива)
    int root;  // Корень (всегда 0)
    int wordID;  // Счётчик ID слов

public:
    Aho() : size(0), root(0), wordID(0) {
        // Выделяем память под 1 узел (корень)
        trie = new Vertex[1];
        // Инициализируем корень
        memset(trie[0].children, -1, sizeof(trie[0].children));  // Все дети = -1
        trie[0].parent = -1;
        trie[0].suffix_link = 0;
        trie[0].end_word_link = 0;
        trie[0].leaf = false;
        trie[0].word_length = 0;
        size = 1;
    }

    ~Aho() {
        delete[] trie;  
    }

    // Добавление подстроки в бор 
    void add_string(const char* s) {
        if (s == nullptr || *s == '\0') return;  
        int curVertex = root;
        int slen = 0;
        for (int i = 0; s[i] != '\0'; ++i) {
            char c = s[i];
            int c_idx = (unsigned char)c;  
            if (trie[curVertex].children[c_idx] == -1) {
                Vertex* new_trie = new Vertex[size + 1];  // Создаём новый узе
                memcpy(new_trie, trie, size * sizeof(Vertex));  // Копируем старые
                delete[] trie;  // Освобождаем старый
                trie = new_trie;
                // Инициализируем новый узел
                memset(trie[size].children, -1, sizeof(trie[size].children));
                trie[size].parent = curVertex;
                trie[size].parent_char = c;
                trie[size].suffix_link = -1;
                trie[size].end_word_link = -1;
                trie[size].leaf = false;
                trie[size].word_length = 0;
                trie[curVertex].children[c_idx] = size;
                ++size;
            }
            curVertex = trie[curVertex].children[c_idx];
            ++slen;
        }
        // Маркируем конец слова
        trie[curVertex].leaf = true;
        trie[curVertex].word_ID = wordID++;
        trie[curVertex].word_length = slen;
    }

    // Построение суффиксных ссылок (prepare автомат)
    void prepare() {
        std::queue<int> vertexQueue;
        vertexQueue.push(root);
        while (!vertexQueue.empty()) {
            int curVertex = vertexQueue.front();
            vertexQueue.pop();
            CalcSuffLink(curVertex);
            for (int i = 0; i < 256; ++i) {
                if (trie[curVertex].children[i] != -1) {
                    vertexQueue.push(trie[curVertex].children[i]);
                }
            }
        }
    }

    // Поиск первого вхождения в текст. Возвращает индекс или -1
    int find_first(const char* text, int text_len, int start_index = 0) const {
        if (text == nullptr || text_len <= start_index) return -1;

        int currentState = root;
        int first_match = -1;

        for (int j = start_index; j < text_len; ++j) {
            char ch = text[j];
            int c_idx = (unsigned char)ch;  

            // Переход по автомату
            while (true) {
                if (trie[currentState].children[c_idx] != -1) {
                    currentState = trie[currentState].children[c_idx];
                    break;
                }
                if (currentState == root) break;
                currentState = trie[currentState].suffix_link;
            }

            // Проверяем матчи по end_word_link (цепочка листьев)
            int checkState = currentState;
            while (checkState != root) {
                if (trie[checkState].leaf) {
                    // Нашли матч: позиция начала
                    int match_index = j - trie[checkState].word_length + 1;
                    if (match_index >= start_index && (first_match == -1 || match_index < first_match)) {
                        first_match = match_index;
                    }
                }
                checkState = trie[checkState].suffix_link;
                // Если это end_word_link, проверим и его (для цепочки)
                if (trie[currentState].end_word_link != root && trie[trie[currentState].end_word_link].leaf) {
                    int end_link_state = trie[currentState].end_word_link;
                    int match_index = j - trie[end_link_state].word_length + 1;
                    if (match_index >= start_index && (first_match == -1 || match_index < first_match)) {
                        first_match = match_index;
                    }
                }
                if (first_match != -1) return first_match;  
            }
        }
        return first_match;
    }

private:
    // Вычисление суффиксной ссылки для узла
    void CalcSuffLink(int vertex) {
        if (vertex == root) {
            trie[vertex].suffix_link = root;
            trie[vertex].end_word_link = root;
            return;
        }
        if (trie[vertex].parent == root) {
            trie[vertex].suffix_link = root;
            trie[vertex].end_word_link = trie[vertex].leaf ? vertex : trie[trie[vertex].suffix_link].end_word_link;
            return;
        }
        int curBetterVertex = trie[trie[vertex].parent].suffix_link;
        int ch_idx = (unsigned char)trie[vertex].parent_char;
        while (true) {
            if (trie[curBetterVertex].children[ch_idx] != -1) {
                trie[vertex].suffix_link = trie[curBetterVertex].children[ch_idx];
                break;
            }
            if (curBetterVertex == root) {
                trie[vertex].suffix_link = root;
                break;
            }
            curBetterVertex = trie[curBetterVertex].suffix_link;
        }
        trie[vertex].end_word_link = trie[vertex].leaf ? vertex : trie[trie[vertex].suffix_link].end_word_link;
    }
};

#endif 