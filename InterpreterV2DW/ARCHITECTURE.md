# Архитектура интерпретатора **AkramovLang++**

## 1. Общая схема обработки

1. **Лексический анализ (`lexer.cpp`)** — функция `get_lexem()` превращает поток символов в токены, вызывая `read_word`, `read_number` или `read_operator` после классификации первого символа, таблицу классификации готовит `make_alphabet`.
2. **Парсинг и семантика (`parser.cpp`)** — метод `semantic_analis()` запускает рекурсивный спуск, начиная с объявлений `DECLAR()` и цепочки операторов `COMMANDS()`/`COMMAND()`; по‑ходу формируется вектор **ПОЛИЗ**.
3. **Генерация ПОЛИЗ** — грамматические функции и управляющие `IF`, `WHILE`, `FOR`, `CONTINUE_BREAK`, `GOTO` добавляют вектор лексем и временные «заглушки» адресов переходов.
4. **Исполнение (`executor.cpp`)** — метод `execute()` последовательно выполняет ПОЛИЗ при помощи стековой машины и функций `oneparam_oper_execution` / `twoparam_oper_execution`.

## 2. Ключевые структуры данных

| Структура | Назначение                                                            |
| --------- | --------------------------------------------------------------------- |
| `Lexema`  | Универсальный дескриптор элемента (тип `lexema_type` + параметр `i`)  |
| `Var`     | Запись о переменной/константе: тип `value_type`, флаг и значение      |
| `Id`      | Элемент `T_id`, связывает строковое имя с записью в `T_var` или метке |

Таблицы `T_var`, `T_const`, `T_id` объявлены в `interpreter.h`.

## 3. Поток управления программы

```
main.cpp
└── Interpretator::analyze()          // запуск обработки
    ├── make_alphabet()              // подготовка алфавита
    ├── semantic_analis()            // парсер + семантика
    │   ├── DECLAR()                 // объявления
    │   └── COMMANDS()               // операторный блок
    └── execute()                    // исполнение ПОЛИЗа
```

## 4. Подробности по исходным файлам

### 4.1 `lexer.cpp`

| Функция           | Роль                                           |
| ----------------- | ---------------------------------------------- |
| `make_alphabet()` | Заполняет классификатор символов               |
| `get_lexem()`     | Главный цикл сканирования                      |
| `read_word()`     | Считывает идентификатор или ключевое слово     |
| `read_number()`   | Считывает числовой литерал и валидирует формат |
| `read_operator()` | Разбирает одно‑/двухсимвольные операторы       |

### 4.2 `parser.cpp`

| Функция             | Роль                                                                          |
| ------------------- | ----------------------------------------------------------------------------- |
| `semantic_analis()` | Контролирует структуру программы, запускает анализ                            |
| `DECLAR()`          | Обрабатывает объявления, создаёт записи через `make_new_var()`                |
| `COMMAND()`         | Диспетчер операторов `IF`, `WHILE`, `FOR`, `READ_WRITE`…                      |
| `VALUE*()`          | Семейство функций для выражений; генерирует ПОЛИЗ и возвращает тип результата |

### 4.3 `poliz.cpp` (управляющие конструкции)

| Функция              | Роль                                                     |
| -------------------- | -------------------------------------------------------- |
| `IF()`               | Создаёт условные переходы `jF`/`jmp` и фиксирует адреса  |
| `WHILE()`            | Формирует цикл, обрабатывает `break/continue` через стек |
| `FOR()`              | Генерирует три секции цикла и точки перехода             |
| `GOTO()` и связанные | Управляют метками и поздним разрешением адресов          |

### 4.4 `executor.cpp`

| Функция                     | Роль                                                        |
| --------------------------- | ----------------------------------------------------------- |
| `execute()`                 | Проходит по ПОЛИЗ, направляет в `oneparam_*` / `twoparam_*` |
| `oneparam_oper_execution()` | Выполняет унарные операции, `read`, `write`                 |
| `twoparam_oper_execution()` | Выполняет бинарные операции, присваивание, сравнения        |

## 5. Таблицы символов

| Таблица   | Содержит                                     | Применение                              |
| --------- | -------------------------------------------- | --------------------------------------- |
| `T_var`   | Все переменные и их значения                 | Чтение/запись при исполнении            |
| `T_const` | Уникальные константы                         | Избегает дублирования литералов в ПОЛИЗ |
| `T_id`    | Идентификаторы → ссылки на `T_var`/`T_label` | Поиск имён при разборе                  |

За управление записью и поиском отвечают `make_new_var()`, `find_in_T_var()`, `find_in_T_const*()`.

## 6. Жизненный цикл выполнения программы AkramovLang++

1. **Компиляция** (`analyze()`): файл → токены → семантика → ПОЛИЗ.
2. **Исполнение** (`execute()`): стековая машина интерпретирует ПОЛИЗ, обновляет `T_var`, выводит результаты.
3. По завершении деструктор `Interpretator` освобождает ресурсы.

---

Документ описывает назначение ключевых модулей и функций и должен служить справочником для разработки и расширения AkramovLang++.
