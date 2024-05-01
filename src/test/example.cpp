

QString word = "привет";
bool correct;
QStringList suggestions = suggestCorrections(word, correct);

if (!correct) {
    qDebug() << "Слово '" << word << "' некорректно написано.";
    qDebug() << "Варианты замены:";
    for (const QString &suggestion : suggestions) {
        qDebug() << "- " << suggestion;
    }
} else {
    qDebug() << "Слово '" << word << "' написано корректно.";
}
