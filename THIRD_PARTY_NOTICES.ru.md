# Сторонние компоненты

Собственный код проекта, скрипты, документация и примеры распространяются
по **MIT License**: Copyright (c) 2026 Artyom Nikiforov (rhyots).
Стандартный текст MIT находится в корневом `LICENSE`
([источник текста](https://opensource.org/license/mit)).

Эта лицензия не заменяет условия сторонних компонентов, перечисленных ниже.
Их исходные уведомления и полные лицензии сохраняются в исходном и бинарных
архивах. В частности, заголовки Notepad++ не перелицензируются под MIT.

## Notepad++ SDK

Заголовки в `vendor/npp-sdk/` взяты из официального репозитория
[notepad-plus-plus/notepad-plus-plus](https://github.com/notepad-plus-plus/notepad-plus-plus),
тег **v8.9.8.1**, коммит **dd40fe45598791e9aa38f3aeb200e10fb3d6338a**.
Исходные уведомления в заголовках сохранены.

- Заголовки API Notepad++ — GPL-3.0-or-later; полный текст находится в
  `vendor/LICENSE.Notepad++.txt`.
- Интерфейсы Scintilla и определения Lexilla — разрешительные лицензии Neil Hodgson
  и других участников; тексты в `vendor/LICENSE.Scintilla.txt` и
  `vendor/LICENSE.Lexilla.txt`.

Точные пути, URL и SHA-256 каждого файла — в `vendor/provenance.json`.
Изменение окончаний строк сторонних файлов отключено через `.gitattributes`.

## Компилятор и стандартная библиотека C++

Проверенная DLL собрана **Zig 0.13.0** с поставляемыми им компонентами LLVM
**libc++**, **libc++abi**, **libunwind**. Уведомления этих компонентов сохранены в:

- `vendor/LICENSE.libcxx.txt`;
- `vendor/LICENSE.libcxxabi.txt`;
- `vendor/LICENSE.libunwind.txt`.

Они содержат Apache License 2.0 с LLVM exceptions и относящиеся к компонентам
исторические уведомления. Компилятор и его исходники в архив проекта не включены.
Для самостоятельной сборки требуется установленный Zig либо CMake и совместимый
компилятор Windows x64. Исходники плагина Notepad++ для Foresight Platform Fore поставляются
отдельным полным архивом `NotepadPlusPlus-Syntax-Plugin-Foresight-Platform-Fore-1.0.0-source.zip` рядом с бинарным архивом.

## Справка Prognoz Platform Fore и дополнительные примеры

Исходная документация Prognoz Platform 8.1 не включается в проект. Сохранены
относительные пути, контрольные суммы и перечни ключевых слов и типов.
Права на платформу, документацию и названия продуктов сохраняются у правообладателей.

Три первоначальных сторонних исходника GitHub использовались как локальные входные
данные теста и не распространяются. Отдельно включён лицензированный фрагмент, указанный ниже.
Собственные примеры в `examples/` написаны для тестирования этого проекта.

## Пример сообщества для документации

`examples/third-party/mcp-data-sources.fore` — фрагмент строк 17–60 модуля
`fap_component/mod_mcp_server.fore` из ForesightPlatform/fap_mcp_server,
коммит `cd6e589860225ee6201aadb603d4c586e5e5ea5d`.
Copyright (c) 2026 Evgeny Bogdanov, MIT. Исходная лицензия сохранена рядом:
`examples/third-party/LICENSE.fap_mcp_server.txt`. Код включён в исходный архив
и показан в визуальной документации. Происхождение: `docs/media/provenance.json`.
