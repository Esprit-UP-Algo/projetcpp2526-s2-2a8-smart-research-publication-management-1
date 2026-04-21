#ifndef UPLOAD_H
#define UPLOAD_H

#include <QString>
#include <QMap>

class PDFUploadHandler
{
public:
    PDFUploadHandler();
    
    bool processUploadedFile(const QString& filePath);
    QMap<QString, QString> getExtractedFields() const;
    QString getLastError() const;

private:
    QString extractTextFromPDF(const QString& filePath);
    QMap<QString, QString> parseExtractedText(const QString& text);
    QString normalizeStatut(const QString& rawStatut);
    
    QMap<QString, QString> m_extractedFields;
    QString m_lastError;
};

#endif // UPLOAD_H