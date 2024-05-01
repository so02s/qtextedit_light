#include <QCompleter>
#include <QTextCodec>
#include <QAction>
#include <QMenu>
#include <QDebug>
#include "./classes.h"

//Описанная чуть выше функция 
QStringList suggestCorrections(const QString &word, bool &correct, bool getSuggests = false)
{
    static Hunspell * m_hunSpell = new Hunspell("/usr/share/hunspell/ru_RU.aff", "/usr/share/hunspell/ru_RU.dic");
    static QString  encoderStr = QString::fromLatin1( m_hunSpell->get_dic_encoding());
    static QTextCodec * m_codec = QTextCodec::codecForName(encoderStr.toLatin1().constData());

    correct = m_hunSpell->spell(m_codec->fromUnicode(word).constData()) != 0;
    if (getSuggests == false)
        return QStringList();
    QStringList suggestions;
    char **suggestWordList = NULL;

    try {
        // Encode from Unicode to the encoding used by current dictionary
        int count = m_hunSpell->suggest(&suggestWordList, m_codec->fromUnicode(word).constData());
        QString lowerWord = word.toLower();

        for (int i = 0; i < count; ++i) {
            QString suggestion = m_codec->toUnicode(suggestWordList[i]);
            suggestions << suggestion;
            free(suggestWordList[i]);
        }
    }
    catch(...)
    {
        qDebug() <<"Error keyword";
    }
    return suggestions;
}

// Переопределяем функцию подсветки текста
void SpellingHighlighter::highlightBlock(const QString &text)
{
    QStringList list = text.split(QRegExp("\\s+"), QString::KeepEmptyParts);//Регулярное выражения по поиску слов
    QTextCharFormat spellingFormat;//Определяем как именно мы будем подсвечивать слова
    spellingFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    spellingFormat.setUnderlineColor(Qt::red);
    int count_word = list.size();
    int pointer = 0;
    for (int i= 0; i< count_word; i++)
    {
        bool correct = false;
        QString sect = text.section(QRegExp("\\s+"), i, i, QString::SectionIncludeLeadingSep);
        sect.remove(QRegExp("[,!?&*|]"));// Удаляем лишние символы
        suggestCorrections(sect, correct);//Проверяем корректно ли слово
        if (!correct)
        {
            setFormat(pointer, sect.length(),spellingFormat);
        }
        pointer += sect.length();
    }
};


BodyTextEdit::BodyTextEdit(QWidget* parent )
:
QTextEdit(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    m_highLighter = new SpellingHighlighter(this);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(openCustomMenu(QPoint)));
}

//Создание контекстного меню с предложенными вариантами замены
void BodyTextEdit::openCustomMenu(QPoint pos)
{
    QMenu* popupmenu = this->createStandardContextMenu();
    QTextCursor cursor = this->textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    this->setTextCursor(cursor);
    if (this->textCursor().hasSelection())
    {
        QString text = this->textCursor().selectedText();
        bool correct = false;
        QStringList suggest = suggestCorrections(text, correct, true);
        auto firstAction = popupmenu->actions().first();
        if (!correct)
        {
            QList<QAction*> addedActions;
            for (auto word  : suggest)
            {
                QAction * act = new QAction(word, popupmenu);
                act->setData(word);
                addedActions.append(act);
            }
            popupmenu->insertActions(firstAction, addedActions);
            connect(popupmenu, SIGNAL(triggered(QAction*)), this, SLOT(correctWord(QAction*)));
        }
    }
    popupmenu->exec(this->mapToGlobal(pos));
    delete popupmenu;
}

//Замена слова на выбранное в меню
void BodyTextEdit::correctWord(QAction *act)
{
    if (act->data().isNull())
        return;
    QString word = act->data().toString();
    QTextCursor cursor = this->textCursor();
    cursor.beginEditBlock();

    cursor.removeSelectedText();
    cursor.insertText(word);
    cursor.endEditBlock();
}
