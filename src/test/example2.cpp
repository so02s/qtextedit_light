BodyTextEdit *textEdit = new BodyTextEdit;
textEdit->setText("Привет мир!");

QTextCursor cursor = textEdit->textCursor();
cursor.select(QTextCursor::WordUnderCursor);
textEdit->setTextCursor(cursor);

textEdit->openCustomMenu(QPoint(100, 100));
