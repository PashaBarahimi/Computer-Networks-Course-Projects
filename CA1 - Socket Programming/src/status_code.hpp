#ifndef STATUS_CODE_HPP_INCLUDE
#define STATUS_CODE_HPP_INCLUDE

struct StatusCode {
    enum type {
        RoomNotFound = 101,
        ReservationNotFound = 102,
        RoomAdded = 104,
        RoomModified = 105,
        RoomDeleted = 106,
        BalanceNotEnough = 108,
        RoomCapacityFull = 109,
        CancelOK = 110,
        RoomExists = 111,
        LoggedOut = 201,
        SignedIn = 230,
        SignedUp = 231,
        UsernameDoesNotExist = 311,
        UserInfoChanged = 312,
        InvalidValue = 401,
        AccessDenied = 403,
        InvalidCapacity = 412,
        UserLeftRoom = 413,
        WrongUserPassword = 430,
        UsernameExists = 451,
        BadCommand = 503,
        OK = 1200,
        BadRequest = 1400,
        Unauthorized = 1401,
    };
};

#endif // STATUS_CODE_HPP_INCLUDE
