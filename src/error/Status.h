#ifndef BSATOOL_STATUS_H
#define BSATOOL_STATUS_H

#include <QString>

using namespace std;

/**
 * @brief Describe the return of a function
 *
 * This class contains an integer indicating the status (negative int indicates
 * an error). In case the status is not zero, the message should be set to
 * detail what happened. The exact meaning of a positive or negative code should
 * be explained by the operation itself
 */
class Status : public exception {
public:
    //**************************************************************************
    // Constructors
    //**************************************************************************
    /**
     * Constructor
     * @param status message code
     * @param message optional explanation
     */
    explicit Status(const int &status, QString message = "");
    //**************************************************************************
    // Getters/setters
    //**************************************************************************
    /**
     * status code
     * @return status code
     */
    [[nodiscard]]int code() const;
    /**
     * message detailing the error if any happened
     * @return message detailing the error if any happened
     */
    [[nodiscard]]QString message() const;
    //**************************************************************************
    // Methods
    //**************************************************************************
    /**
     * std::string version of the message detailing the error if any happened
     * @return std::string version of the message detailing the error if any happened
     */
    [[nodiscard]]const char* what() const noexcept override;

private:
    //**************************************************************************
    // Attributes
    //**************************************************************************
    /**
     * @brief status code.
     */
    int mCode;
    /**
     * @brief message detailing the error if any happened
     */
    QString mMessage;
    /**
     * @brief std::string version of the message detailing the error if any happened
     */
    std::string mMessageStdString;
};

#endif // BSATOOL_STATUS_H
