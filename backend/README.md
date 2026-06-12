# Backend

Backend-часть дипломного проекта Pixio реализована на C++ с использованием фреймворка Drogon.

Backend отвечает за обработку HTTP-запросов, работу с PostgreSQL, авторизацию пользователей, разграничение ролей покупателя и продавца, обработку заказов и основную бизнес-логику marketplace.

## Стек

- C++
- Drogon
- PostgreSQL
- REST API
- JSON
- CMake

## Сборка

```bash
mkdir -p build
cmake -S . -B build
cmake --build build -j2
