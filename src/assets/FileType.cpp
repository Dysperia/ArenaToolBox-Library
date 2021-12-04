#include <assets/FileType.h>

//**************************************************************************
// Constructors
//**************************************************************************
FileType::FileType() = default;

//**************************************************************************
// Methods
//**************************************************************************
QString FileType::getDescriptionForExtension(const FileType::Extension extension) {
    switch (extension) {
        case CFA:
            return "Animation";
        case IMG:
            return "Single image";
        case CIF:
            return "Image set";
        case INF:
            return "Encrypted text";
        case XFM:
            return "Low quality Midi audio";
        case XMI:
            return "High quality Midi audio";
        case VOC:
            return "Audio";
        case DFA:
            return "Image partly animated";
        case MIF:
            return "Interior map";
        case SET:
            return "Texture set";
        case TXT:
            return "Text file";
        case RMD:
            return "Exterior map";
        case UNKNOWN:
        default:
            return "Unknown file type";
    }
}

FileType::Extension FileType::getExtension(const BsaFile &file)
{
    const QString &ext = file.getExtension();
    if (ext.compare("CFA", Qt::CaseInsensitive) == 0) {
        return Extension::CFA;
    }
    else if (ext.compare("IMG", Qt::CaseInsensitive) == 0) {
        return Extension::IMG;
    }
    else if (ext.compare("CIF", Qt::CaseInsensitive) == 0) {
        return Extension::CIF;
    }
    else if (ext.compare("INF", Qt::CaseInsensitive) == 0) {
        return Extension::INF;
    }
    else if (ext.compare("XFM", Qt::CaseInsensitive) == 0) {
        return Extension::XFM;
    }
    else if (ext.compare("XMI", Qt::CaseInsensitive) == 0) {
        return Extension::XMI;
    }
    else if (ext.compare("VOC", Qt::CaseInsensitive) == 0) {
        return Extension::VOC;
    }
    else if (ext.compare("DFA", Qt::CaseInsensitive) == 0) {
        return Extension::DFA;
    }
    else if (ext.compare("MIF", Qt::CaseInsensitive) == 0) {
        return Extension::MIF;
    }
    else if (ext.compare("SET", Qt::CaseInsensitive) == 0) {
        return Extension::SET;
    }
    else if (ext.compare("TXT", Qt::CaseInsensitive) == 0) {
        return Extension::TXT;
    }
    else if (ext.compare("RMD", Qt::CaseInsensitive) == 0) {
        return Extension::RMD;
    }
    else {return Extension::UNKNOWN;}
}
