#include "VaultItem.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <random>

std::string AccountTypeToString(AccountType type) {
    switch (type) {
        case AccountType::Basic: return "Basic App Password";
        case AccountType::SecurityQuestions: return "Security Questions";
        case AccountType::Miscellaneous: return "Miscellaneous";
        default: return "Basic App Password";
    }
}

AccountType StringToAccountType(const std::string& str) {
    if (str == "Security Questions" || str == "SecurityQuestions") return AccountType::SecurityQuestions;
    if (str == "Miscellaneous") return AccountType::Miscellaneous;
    return AccountType::Basic;
}

std::string VaultItem::FormatTimestamp(int64_t ts) {
    std::time_t t = static_cast<std::time_t>(ts);
    std::tm tmBuf;
#if defined(_WIN32)
    localtime_s(&tmBuf, &t);
#else
    localtime_r(&t, &tmBuf);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tmBuf, "%b %d, %Y - %I:%M:%S %p");
    return ss.str();
}

VaultItem::VaultItem()
    : type(AccountType::Basic),
      tabName("General"),
      submittedTimestamp(0),
      sortOrder(0) {
}

VaultItem VaultItem::CreateNew(AccountType type, const std::string& label, const std::string& tab) {
    VaultItem item;
    item.type = type;
    item.label = label;
    item.tabName = tab.empty() ? "General" : tab;
    item.submittedTimestamp = static_cast<int64_t>(std::time(nullptr));
    item.submittedTimeFormatted = FormatTimestamp(item.submittedTimestamp);

    // Generate unique ID
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dis;
    std::ostringstream ss;
    ss << "item_" << item.submittedTimestamp << "_" << std::hex << dis(gen);
    item.id = ss.str();
    item.sortOrder = 0;

    return item;
}

nlohmann::json VaultItem::ToJson() const {
    nlohmann::json j;
    j["id"] = id;
    j["type"] = static_cast<int>(type);
    j["label"] = label;
    j["username"] = username;
    j["email"] = email;
    j["password"] = password;
    j["website"] = website;
    j["notes"] = notes;
    j["totpSecret"] = totpSecret;
    j["tabName"] = tabName;
    j["submittedTimestamp"] = submittedTimestamp;
    j["submittedTimeFormatted"] = submittedTimeFormatted;
    j["sortOrder"] = sortOrder;

    nlohmann::json sqList = nlohmann::json::array();
    for (const auto& sq : securityQuestions) {
        nlohmann::json qObj;
        qObj["question"] = sq.question;
        qObj["answer"] = sq.answer;
        sqList.push_back(qObj);
    }
    j["securityQuestions"] = sqList;

    nlohmann::json cfList = nlohmann::json::array();
    for (const auto& cf : customFields) {
        nlohmann::json fObj;
        fObj["label"] = cf.label;
        fObj["value"] = cf.value;
        cfList.push_back(fObj);
    }
    j["customFields"] = cfList;

    j["attachedImageBase64"] = attachedImageBase64;
    j["attachedImageName"] = attachedImageName;

    return j;
}

VaultItem VaultItem::FromJson(const nlohmann::json& j) {
    VaultItem item;
    item.id = j.value("id", "");
    item.type = static_cast<AccountType>(j.value("type", 0));
    item.label = j.value("label", "");
    item.username = j.value("username", "");
    item.email = j.value("email", "");
    item.password = j.value("password", "");
    item.website = j.value("website", "");
    item.notes = j.value("notes", "");
    item.totpSecret = j.value("totpSecret", "");
    item.tabName = j.value("tabName", "General");
    item.submittedTimestamp = j.value("submittedTimestamp", static_cast<int64_t>(0));
    item.submittedTimeFormatted = j.value("submittedTimeFormatted", "");
    if (item.submittedTimeFormatted.empty() && item.submittedTimestamp > 0) {
        item.submittedTimeFormatted = FormatTimestamp(item.submittedTimestamp);
    }
    item.sortOrder = j.value("sortOrder", 0);
    item.attachedImageBase64 = j.value("attachedImageBase64", "");
    item.attachedImageName = j.value("attachedImageName", "");

    if (j.contains("securityQuestions") && j["securityQuestions"].is_array()) {
        for (const auto& qObj : j["securityQuestions"]) {
            SecurityQuestion sq;
            sq.question = qObj.value("question", "");
            sq.answer = qObj.value("answer", "");
            item.securityQuestions.push_back(sq);
        }
    }

    if (j.contains("customFields") && j["customFields"].is_array()) {
        for (const auto& fObj : j["customFields"]) {
            CustomField cf;
            cf.label = fObj.value("label", "");
            cf.value = fObj.value("value", "");
            item.customFields.push_back(cf);
        }
    }

    return item;
}
