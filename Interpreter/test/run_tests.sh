#!/usr/bin/env bash
# test/run_tests.sh
# Скрипт пробегает по всем *.txt в ../data и запускает на каждом наш интерпретатор.

set -e  # прерываемся при любой ошибке (неуспешный возврат из интерпретатора)

BIN="../bin/interpreter"

if [[ ! -x "$BIN" ]]; then
  echo "Ошибка: не найден исполняемый файл $BIN"
  exit 1
fi

echo
echo "=== Начало тестирования программ из data/ ==="
echo

for src in ../data/*.txt; do
  echo "----------"
  echo "Тест  : $src"
  echo "Вывод :"
  "$BIN" "$src"
  echo
done

echo "=== Все тесты успешно выполнены ==="
