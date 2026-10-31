# Сборка и публикация выпуска

[English](RELEASING.md) | [Русский](RELEASING.ru.md)

Репозиторий: [rhyots/notepad-plus-plus-syntax-plugin-foresight-platform-fore](https://github.com/rhyots/notepad-plus-plus-syntax-plugin-foresight-platform-fore).

## Подготовка

1. Согласуйте версии в CMake, ресурсе, упаковщике и установщике. Обновите оба README и CHANGELOG.
2. Соберите Zig 0.13.0 и выполните `test.ps1`. Для изменённой DLL запустите `qa/run.ps1`, `qa/run-autodetect.ps1`, `qa/run-theme.ps1` и `qa/test-install.ps1` на отдельных копиях проверенной версии Notepad++.
3. Обновляйте `byte-lock.json` только для намеренных изменений. Запустите `tools/check_bytes.py`, `tools/generate_palettes.py --check`, `tools/inspect_binary.py` и Python-тесты. Обновление хешей не заменяет нативный прогон.
4. Выполните `tools/package.py`, затем `qa/test-catalog-install.ps1 -Archive <catalog-zip>` и `tools/catalog_audit.py`. После обновления отчётов пересоздайте исходный архив.
5. Выполните `tools/checkout_audit.py`. Он проверяет настоящие Git staging/checkout с обоими режимами autocrlf, включая отказ при изменении байтов. Затем выполните `tools/package.py --source-only`, чтобы включить новый отчёт.

## Публикация в GitHub

Отправьте проверенный коммит в `main`, дождитесь всех успешных заданий матрицы и поставьте этому же коммиту тег `v1.0.0`. Создайте Release по [RELEASE-NOTES.md](RELEASE-NOTES.md), добавив ссылку на [русское описание](RELEASE-NOTES.ru.md) или включив его в текст. Загрузите три подготовленных ZIP и их `.sha256`; не заменяйте проверенные файлы автоматически создаваемыми исходными архивами GitHub.

После публикации скачайте настоящие файлы выпуска и сравните SHA-256 с локальными. Проверьте ссылки скачивания в README, GIF, переключение языка, лицензию и руководства. Описание и topics должны точно описывать Fore, Foresight, Прогноз, Notepad++ и подсветку; не заявляйте поддержку Fore.NET и непроверенных архитектур.

Если до первого Release потребуется исправление по результатам CI, завершите его и используйте исправленный коммит. Пересоберите исходный ZIP, чтобы он соответствовал публичному дереву тега. Бинарные архивы нужно пересобирать при изменении их входных файлов; при любом изменении каталогового ZIP пересчитывается его хеш.

## Plugins Admin — отдельный этап

Выпуск в GitHub не добавляет плагин в Notepad++ Plugins Admin автоматически. См. [готовность к каталогу](PLUGINS-ADMIN-READINESS.md). Поле `id` — SHA-256 всего `*-plugins-admin-x64.zip`. Проверьте настоящий публичный URL, затем выполните официальную процедуру установки/обновления/удаления в отдельной среде до отправки PR в каталог.

## Визуальная документация

GIF — превью кода, не запись редактора. Соберите `qa/preview-tokens.cpp` вместе с `src/ForeLexer.cpp`, затем используйте `tools/render_previews.py --lexer <helper.exe> --language en` и `--language ru`. Нужен Python с Pillow; текущий визуализатор использует Windows-шрифты Consolas/Segoe UI. Сохраните происхождение кода и исходную MIT-лицензию рядом с фрагментом сообщества. [Происхождение визуализаций](media/README.ru.md).
