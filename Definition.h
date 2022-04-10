#pragma once
#include <functional>
#include <tuple>
#include <memory>
class TCPConnection;

using TCPConnectionPtr = std::shared_ptr<TCPConnection>;

using TCPConnectedCB = std::function<void(const TCPConnectionPtr &conn)>;
using TCPMessageCB = std::function<void(const TCPConnectionPtr &conn, std::vector<char> &&message)>;
using TCPClosingCB = std::function<void(const TCPConnectionPtr &conn)>;
using TCPErrorCB = std::function<void(const TCPConnectionPtr &conn)>;

constexpr int32_t HeaderNotFound = -1;
// return (headerlen,contentlen) < 0 not found
using IFCompleteHeader = std::function<std::pair<int32_t, int32_t>(const char *, int32_t)>;
//(header,content len,extra read) extra maybe the content or maybe another header
using HeaderRet = std::tuple<std::vector<char>, int32_t, std::vector<char>>;