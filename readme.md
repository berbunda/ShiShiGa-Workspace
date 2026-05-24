# ShiShiga Workspace
ShiShiga Workspace — desktop workspace для работы с AI-сервисами через web-интерфейс без использования API TOKENs.

## Цель проекта | The purpose of the project
Основная цель проекта — возможность использовать AI-сервисы в виде отдельной программы, не держа множество вкладок в браузере.  
Проект разрабатывается в первую очередь под личные задачи автора, однако в будущем может быть полезен и другим пользователям.  

---

The main goal of the project is to be able to use AI services as a separate program without having to keep multiple tabs in the browser.  
The project is being developed primarily for the personal tasks of the author, but in the future it may be useful to other users.

## Особенности | Features
- Работа через встроенный QtWebEngine
- Не используются API TOKENs
- Сохранение авторизации между перезапусками
- Отдельные профили для AI-сервисов
- Chromium-based browser engine внутри приложения

---

- Work through the built-in QtWebEngine - API TOKENs are not used - Saving authorization between restarts - Separate profiles for AI services - Chromium-based browser engine inside the application

## Важно | Important
На данный момент проект находится в стадии MVP.
Оптимизация пока не проводилась:
- высокий расход оперативной памяти (~500 MB)
- медленный отклик интерфейса
- возможны баги и нестабильность
После реализации основного функционала будет начата работа над оптимизацией.

---

At the moment, the project is in the MVP stage.  
Optimization has not been carried out yet:
- high RAM consumption (~500 MB)  
- slow interface response  
- bugs and instability are possible
After the implementation of the main functionality, optimization work will begin.

## Linux
Планируется версия для Linux, однако работа над ней начнётся только после стабилизации Windows-версии.

---

A Linux version is planned, but work on it will begin only after the Windows version has stabilized.

## Используемые технологии | Technologies used
- Qt 6.11
- Qt WebEngine
- MSVC 2022
- C++23
- CMake 4.3.3

## Что реализовано
- Открытие ChatGPT внутри Qt-приложения
- Сохранение состояния авторизации
- Persistent Chromium/QWebEngine profiles
- Хранение профилей в `%LOCALAPPDATA%`

## Планируемый функционал | What is implemented
- Вкладки для разных AI-сервисов
- Окно настроек
- Полноценный менеджер профилей
- Очистка профилей
- Поддержка нескольких AI-сервисов
- Боковая панель/система навигации

---

- Tabs for different AI services
- The settings window
- Full-fledged profile manager
- Cleaning profiles
- Support for multiple AI services
- Sidebar/navigation system

## Статус проекта | Project status
Текущий статус:
- Завершен MVP-0
Ниже представлен текущий kanban проекта.

---

Current status:
- MVP-0 completed
The current kanban of the project is shown below.

![[kanban.png]]
