#include <utility>
#include <error/Status.h>

//**************************************************************************
// Constructors
//**************************************************************************
Status::Status(const int &code, QString message) :
        mCode(code), mMessage(move(message)), mMessageStdString(mMessage.toStdString()) {

}

//**************************************************************************
// Getters/setters
//**************************************************************************
int Status::code() const {
    return mCode;
}

QString Status::message() const {
    return mMessage;
}

const char *Status::what() const noexcept {
    return mMessageStdString.data();
}
