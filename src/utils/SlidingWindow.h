#ifndef BSATOOL_SLIDINGWINDOW_H
#define BSATOOL_SLIDINGWINDOW_H

#include <cstdlib>
#include <array>
#include <QVector>
#include <QMultiMap>

using namespace std;

/**
 * The sliding window is a length fixed vue on the current uncompressed data. It should be moved after each
 * processed or read data. The value at the current insertion position is the data at the beginning of the window and
 * the data before is the last read or processed.
 * The window cycles its current index position each sw_size, rewriting old data with new one, hence the sliding.
 *
 * In order to fasten duplicate search, the window use an internal dictionary. While this has a positive effect
 * on the compression side of an algorithm, it slows the uncompression by updating a useless dictionary. Therefore, it
 * is encouraged to use the dictionary (active by default) for compression but set it inactive for uncompression.
 * The duplicate search use a full window scan if the dictionary is inactive.
 * @tparam sw_type data type to store
 * @tparam sw_size total length of the window
 */
template<typename sw_type, size_t sw_size>
class SlidingWindow {
public:
    //**************************************************************************
    // Structures
    //**************************************************************************
    /**
     * Store the result of a duplicate search in a sliding window
     * Length is the duplicate length and startIndex is the start offset in the
     * sliding window
     */
    struct DuplicateSearchResult {
        size_t length;
        size_t startIndex;
    };

    //**************************************************************************
    // Constructors
    //**************************************************************************
    explicit SlidingWindow(bool useDictionary = true);

    //**************************************************************************
    // Getters/setters
    //**************************************************************************
    /**
     * return the index of the next insertion (index of the oldest data)
     * @return the index between 0 and sw_size - 1
     */
    [[nodiscard]] size_t getMCurrentInsertPosition() const;

    /**
     * return the array used to internally manage the window
     * @return an array of size sw_size and type sw_type
     */
    const array<sw_type, sw_size> &getWindow() const;

    /**
     * @return True if the sliding window is using the internal dictionary
     */
    [[nodiscard]] bool useDictionary() const;

    //**************************************************************************
    // Methods
    //**************************************************************************
    /**
     * Search for a duplicate in the sliding window
     * @param uncompressDataDeque data from which read the ongoing data to insert
     * @param max_duplicate_length max length for a duplicate to copy
     * @return the search result
     */
    DuplicateSearchResult searchDuplicateInSlidingWindow(const deque<sw_type> &uncompressDataDeque,
                                                         size_t max_duplicate_length);

    /**
     * Read the data at a given index in the window
     * @param index Index from which to read. If it is not in the range [0, sw_size-1] it will become using
     * modulo sw_size
     * @return the data at the given index
     */
    const sw_type &readAtIndex(const size_t &index) const;

    /**
     * Insert the given data at the current insertion index, replacing the oldest data. The current insertion index is
     * increased by one
     * @param newValue The value to insert
     */
    void insert(const sw_type &newValue);

    /**
     * Return the lowest positive index equivalent to the given one, in range [0, _size-1]
     * @param index to get the standard from
     * @return the standard index
     */
    [[nodiscard]] size_t getStandardEquivalentIndex(const size_t &index) const;

private:
    //**************************************************************************
    // Attributes
    //**************************************************************************
    /**
     * Index of the oldest data, the next to be replace
     */
    size_t mCurrentInsertPosition = 0;
    /**
     * Array used to internally build the sliding window
     */
    array<sw_type, sw_size> mWindow{};

    /**
     * Map used to store duplicate indexes based on the three first elements (constituting the map key)
     */
    QMultiMap<string, size_t> mDuplicateDictionary{};

    /**
     * True (default) to allow the sliding window to use the internal dictionary
     */
    bool mUseDictionary{true};

    //**************************************************************************
    // Methods
    //**************************************************************************
    /**
     * Search for a duplicate in the possibly soon rewritten part of the sliding window
     * @param uncompressDataDeque data from which read the ongoing data to insert
     * @param max_duplicate_length max length for a duplicate to copy
     * @return the search result
     */
    DuplicateSearchResult searchDuplicateInSlidingWindowLookAheadOnly(const deque<sw_type> &uncompressDataDeque,
                                                                      size_t max_duplicate_length);

    /**
     * Search for a duplicate in the sliding window, avoiding the last max_duplicate_length bytes of the window
     * @param uncompressDataDeque data from which read the ongoing data to insert
     * @param max_duplicate_length max length for a duplicate to copy
     * @return the search result
     */
    DuplicateSearchResult searchDuplicateInSlidingWindowNoLookAhead(const deque<sw_type> &uncompressDataDeque,
                                                                    size_t max_duplicate_length);
};


//**************************************************************************
// Definitions
//**************************************************************************

// Constructors

template<typename sw_type, size_t sw_size>
SlidingWindow<sw_type, sw_size>::SlidingWindow(bool useDictionary): mUseDictionary(useDictionary) {}

// Getters/setters

template<typename sw_type, size_t sw_size>
size_t SlidingWindow<sw_type, sw_size>::getMCurrentInsertPosition() const {
    return mCurrentInsertPosition;
}

template<typename sw_type, size_t sw_size>
const array<sw_type, sw_size> &SlidingWindow<sw_type, sw_size>::getWindow() const {
    return mWindow;
}

template<typename sw_type, size_t sw_size>
bool SlidingWindow<sw_type, sw_size>::useDictionary() const {
    return mUseDictionary;
}

// Methods

template<typename sw_type, size_t sw_size>
const sw_type &SlidingWindow<sw_type, sw_size>::readAtIndex(const size_t &index) const {
    return mWindow[getStandardEquivalentIndex(index)];
}

template<typename sw_type, size_t sw_size>
void SlidingWindow<sw_type, sw_size>::insert(const sw_type &newValue) {
    size_t idx(mCurrentInsertPosition);
    if (mUseDictionary) {
        mDuplicateDictionary.insert({newValue, mWindow[idx + 1], mWindow[idx + 2]}, idx);
        mDuplicateDictionary.remove({mWindow[idx], mWindow[idx + 1], mWindow[idx + 2]}, idx);
    }
    mWindow[idx] = newValue;
    mCurrentInsertPosition = (idx + 1) % sw_size;
}

template<typename sw_type, size_t sw_size>
size_t SlidingWindow<sw_type, sw_size>::getStandardEquivalentIndex(const size_t &index) const {
    return index % sw_size;
}

template<typename sw_type, size_t sw_size>
typename SlidingWindow<sw_type, sw_size>::DuplicateSearchResult SlidingWindow<sw_type, sw_size>::searchDuplicateInSlidingWindowLookAheadOnly(
        const deque<sw_type> &uncompressDataDeque, const size_t max_duplicate_length) {
    // search longest possible considering max duplicate length and remaining uncompressed data
    quint8 max_possible_duplicate_length(
            uncompressDataDeque.size() < max_duplicate_length ? uncompressDataDeque.size() : max_duplicate_length);
    // building preview window using current data and future one
    QVector<char> snapshotFutureWindow;
    // end of current buffer
    for (int i(-int(max_duplicate_length)); i < 0; ++i) {
        snapshotFutureWindow.push_back(readAtIndex(getMCurrentInsertPosition() + i));
    }
    // data that will next be written in buffer
    for (size_t i(0); i < max_possible_duplicate_length - 1; ++i) {
        snapshotFutureWindow.push_back(uncompressDataDeque[i]);
    }
    // searching for duplicate
    DuplicateSearchResult result = {0, 0};
    const char &nextUncompressedByte = uncompressDataDeque.front();
    for (int i(0); i < max_duplicate_length && result.length < max_possible_duplicate_length; ++i) {
        // found start for a match
        if (nextUncompressedByte == snapshotFutureWindow[i]) {
            // computing sequence length
            quint8 tempLength(1);
            // computing while not the longest length and available data
            while (tempLength < max_possible_duplicate_length &&
                   snapshotFutureWindow[i + tempLength] == uncompressDataDeque[tempLength]) {
                tempLength++;
            }
            // writing result if longer
            if (tempLength > result.length) {
                result.length = tempLength;
                result.startIndex = getStandardEquivalentIndex(getMCurrentInsertPosition() + i - max_duplicate_length);
            }
        }
    }
    return result;
}

template<typename sw_type, size_t sw_size>
typename SlidingWindow<sw_type, sw_size>::DuplicateSearchResult SlidingWindow<sw_type, sw_size>::searchDuplicateInSlidingWindowNoLookAhead(
        const deque<sw_type> &uncompressDataDeque, const size_t max_duplicate_length) {
    DuplicateSearchResult result = {0, 0};
    quint8 tempLength;
    quint16 tempStartIndex;
    // If not at least 3 elements in incoming data, stop. Only duplicate of length 3 or more are searched
    if (uncompressDataDeque.size() >= 3) {
        if (mUseDictionary) {
            auto threeElemDuplicIdx = mDuplicateDictionary.values({uncompressDataDeque[0], uncompressDataDeque[1], uncompressDataDeque[2]});
            const char &nextUncompressedByte = uncompressDataDeque.front();
            for (int i = 0; i < threeElemDuplicIdx.size() && result.length < max_duplicate_length; ++i) {
                tempStartIndex = threeElemDuplicIdx.at(i);
                // If not idx that should be search in lookahead
                if (getStandardEquivalentIndex(getMCurrentInsertPosition() - tempStartIndex) > 18) {
                    // Found a possible match
                    if (nextUncompressedByte == readAtIndex(tempStartIndex)) {
                        tempLength = 1;
                        // computing length for the found match, while checking if enough data available for it
                        while (tempLength < uncompressDataDeque.size() &&
                               tempLength < max_duplicate_length &&
                               uncompressDataDeque[tempLength] == readAtIndex(tempStartIndex + tempLength)) {
                            tempLength++;
                        }
                        // keeping only if longer than a previous one
                        if (tempLength > result.length) {
                            result.length = tempLength;
                            result.startIndex = tempStartIndex;
                        }
                    }
                }
            }
        } else {
            // searching a first byte match until longest found or all window searched
            // starting at offset 1 from current position to avoid the window current index
            const char &nextUncompressedByte = uncompressDataDeque.front();
            for (int i = 1; i < 4096 - max_duplicate_length && result.length < max_duplicate_length; ++i) {
                tempStartIndex = getStandardEquivalentIndex(getMCurrentInsertPosition() + i);
                // Found a possible match
                if (nextUncompressedByte == readAtIndex(tempStartIndex)) {
                    tempLength = 1;
                    // computing length for the found match, while checking if enough data available for it
                    while (tempLength < uncompressDataDeque.size() &&
                           tempLength < max_duplicate_length &&
                           uncompressDataDeque[tempLength] == readAtIndex(tempStartIndex + tempLength)) {
                        tempLength++;
                    }
                    // keeping only if longer than a previous one
                    if (tempLength > result.length) {
                        result.length = tempLength;
                        result.startIndex = tempStartIndex;
                    }
                }
            }
        }
    }
    return result;
}

template<typename sw_type, size_t sw_size>
typename SlidingWindow<sw_type, sw_size>::DuplicateSearchResult SlidingWindow<sw_type, sw_size>::searchDuplicateInSlidingWindow(const deque<sw_type> &uncompressDataDeque,
                                                                                                                                const size_t max_duplicate_length) {
    // searching for an ongoing duplicate using the possibly rewritten part of the window
    const DuplicateSearchResult lookAhead = searchDuplicateInSlidingWindowLookAheadOnly(uncompressDataDeque,
                                                                                        max_duplicate_length);
    DuplicateSearchResult noLookAhead = {0, 0};
    // not longest found
    if (lookAhead.length < max_duplicate_length) {
        // Search through buffer in case there is a longer duplicate to copy avoiding the possibly
        // rewritten section already search before
        noLookAhead = searchDuplicateInSlidingWindowNoLookAhead(uncompressDataDeque, max_duplicate_length);
    }
    return lookAhead.length > noLookAhead.length ? lookAhead : noLookAhead;
}


#endif // BSATOOL_SLIDINGWINDOW_H
