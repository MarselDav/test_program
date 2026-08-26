#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

enum class DuplicateAction
{
    Counter,
    Rewrite
};

enum class Mode
{
    Single,
    Timer
};

class Settings
{
public:
    Settings(QString m = "",
             bool dIFile = false,
             QString sP = "",
             QString sN = "",
             QString inP = "",
             DuplicateAction dA = DuplicateAction::Counter,
             Mode wM = Mode::Single,
             int s = 0,
             QString key = "")
        : mask(m),
        deleteInputFile(dIFile),
        savePath(sP),
        saveName(sN),
        inputPath(inP),
        duplicateAction(dA),
        workMode(wM),
        seconds(s),
        XOR_key(key)
    {}

    QString mask;
    bool deleteInputFile;
    QString savePath;
    QString saveName;
    QString inputPath;
    DuplicateAction duplicateAction;
    Mode workMode;
    int seconds;
    QString XOR_key;
};

#endif // SETTINGS_H
