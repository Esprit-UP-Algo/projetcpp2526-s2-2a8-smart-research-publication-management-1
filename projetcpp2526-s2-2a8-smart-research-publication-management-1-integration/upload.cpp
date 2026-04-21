#include "upload.h"
#include <QProcess>
#include <QDebug>
#include <QFileInfo>
#include <QStringList>
#include <iostream>

PDFUploadHandler::PDFUploadHandler()
{
}

bool PDFUploadHandler::processUploadedFile(const QString& filePath)
{
    m_extractedFields.clear();
    m_lastError.clear();
    
    qDebug() << "Starting PDF processing for:" << filePath;
    
    // Extract text from PDF using pdftotext
    QString extractedText = extractTextFromPDF(filePath);
    if (extractedText.isEmpty()) {
        return false;
    }
    
    qDebug() << "Raw text length extracted:" << extractedText.length();
    
    // Parse the extracted text
    m_extractedFields = parseExtractedText(extractedText);
    
    // If no fields found, use filename as title
    if (m_extractedFields.isEmpty()) {
        QFileInfo fileInfo(filePath);
        QString filename = fileInfo.baseName();
        m_extractedFields["titre"] = filename;
        qDebug() << "No fields found, using filename as title:" << filename;
    }
    
    // Debug output of final field map
    qDebug() << "Final extracted fields:";
    for (auto it = m_extractedFields.begin(); it != m_extractedFields.end(); ++it) {
        qDebug() << "  " << it.key() << ":" << it.value();
    }
    
    return true;
}

QString PDFUploadHandler::extractTextFromPDF(const QString& filePath)
{
    QProcess process;
    
    // Use pdftotext with -layout flag and output to stdout
    QStringList arguments;
    arguments << "-layout" << filePath << "-";
    
    qDebug() << "Starting QProcess with pdftotext";
    process.start("pdftotext", arguments);
    
    if (!process.waitForStarted(5000)) {
        m_lastError = "Cet outil nécessite pdftotext. Téléchargez Poppler for Windows, extrayez pdftotext.exe et ajoutez-le au PATH système.";
        qDebug() << "Failed to start pdftotext process";
        return QString();
    }
    
    if (!process.waitForFinished(10000)) {
        m_lastError = "Le processus pdftotext a expiré.";
        qDebug() << "pdftotext process timed out";
        return QString();
    }
    
    if (process.exitCode() != 0) {
        m_lastError = "Erreur lors de l'extraction du texte PDF: " + process.readAllStandardError();
        qDebug() << "pdftotext failed with exit code:" << process.exitCode();
        return QString();
    }
    
    QByteArray output = process.readAllStandardOutput();
    QString text = QString::fromUtf8(output);
    
    return text;
}

QMap<QString, QString> PDFUploadHandler::parseExtractedText(const QString& text)
{
    QMap<QString, QString> fields;
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);
    
    for (const QString& line : lines) {
        QString trimmedLine = line.trimmed().toLower();
        
        // Skip lines containing author-related keywords
        if (trimmedLine.contains("auteurs") || trimmedLine.contains("authors") || 
            trimmedLine.contains("auteur") || trimmedLine.contains("author")) {
            continue;
        }
        
        // Look for field patterns with various separators
        QStringList separators = {":", "=", "-"};
        
        for (const QString& separator : separators) {
            if (trimmedLine.contains(separator)) {
                QStringList parts = trimmedLine.split(separator, Qt::SkipEmptyParts);
                if (parts.size() >= 2) {
                    QString key = parts[0].trimmed();
                    QString value = parts.mid(1).join(separator).trimmed();
                    
                    // Map field patterns to canonical keys
                    if (key.contains("titre") || key.contains("title") || key.contains("nom")) {
                        fields["titre"] = value;
                        qDebug() << "Found field - titre:" << value;
                    }
                    else if (key.contains("date de publication") || key.contains("date publication") || key.contains("date")) {
                        fields["date_de_publication"] = value;
                        qDebug() << "Found field - date_de_publication:" << value;
                    }
                    else if (key.contains("revue/journal") || key.contains("revue") || key.contains("journal")) {
                        fields["revue_journal"] = value;
                        qDebug() << "Found field - revue_journal:" << value;
                    }
                    else if (key.contains("statut") || key.contains("status") || key.contains("état") || key.contains("etat")) {
                        QString normalizedStatut = normalizeStatut(value);
                        if (!normalizedStatut.isEmpty()) {
                            fields["statut"] = normalizedStatut;
                            qDebug() << "Found field - statut:" << normalizedStatut << "(from:" << value << ")";
                        }
                    }
                }
                break; // Found a separator, no need to check others for this line
            }
        }
    }
    
    return fields;
}

QString PDFUploadHandler::normalizeStatut(const QString& rawStatut)
{
    QString normalized = rawStatut.toLower().trimmed();
    
    // Remove accents for fuzzy matching
    normalized = normalized.replace("é", "e").replace("è", "e").replace("ê", "e");
    normalized = normalized.replace("à", "a").replace("â", "a");
    normalized = normalized.replace("ù", "u").replace("û", "u");
    normalized = normalized.replace("ô", "o").replace("ö", "o");
    normalized = normalized.replace("î", "i").replace("ï", "i");
    normalized = normalized.replace("ç", "c");
    
    // Map to canonical statut values
    if (normalized.contains("accepte") || normalized.contains("accepted")) {
        return "accepté";
    }
    else if (normalized.contains("publie") || normalized.contains("published")) {
        return "publié";
    }
    else if (normalized.contains("rejete") || normalized.contains("rejected")) {
        return "rejeté";
    }
    else if (normalized.contains("revision") || normalized.contains("review")) {
        return "en révision";
    }
    else if (normalized.contains("soumis") || normalized.contains("submitted")) {
        return "soumis";
    }
    
    return QString(); // No match found
}

QMap<QString, QString> PDFUploadHandler::getExtractedFields() const
{
    return m_extractedFields;
}

QString PDFUploadHandler::getLastError() const
{
    return m_lastError;
}