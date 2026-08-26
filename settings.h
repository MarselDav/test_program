#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

enum onDuplicateInputFileAction
{
    Counter,
    Rewrite
};

enum Mode
{
    Single,
    Timer
};

class Settings
{
public:
    Settings(QString m, bool dIFile,
             QString sP, QString sN, QString inP,
             int dA, int wM,
             int s, QString key) : mask(m),
        deleteInputFile(dIFile),
        savePath(sP), saveName(sN), inputPath(inP), duplicateAction(dA),
        workMode(wM), seconds(s), XOR_key(key) {}

    QString mask;
    bool deleteInputFile;
    QString savePath;
    QString saveName;
    QString inputPath;
    int duplicateAction;
    int workMode;
    int seconds;
    QString XOR_key;
};

#endif // SETTINGS_H
