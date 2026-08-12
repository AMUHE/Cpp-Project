#pragma once

#include <QString>

namespace saw::logging {

class JsonLogger {
public:
    static bool install(const QString &directory, QString *error = nullptr);
    static void shutdown();
};

} // namespace saw::logging
