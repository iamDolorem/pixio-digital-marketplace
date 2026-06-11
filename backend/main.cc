#include <drogon/drogon.h>
#include <drogon/orm/DbClient.h>
#include <drogon/orm/Exception.h>
#include <limits>
#include <vector>
#include <random>

using namespace drogon;

void addCorsHeaders(const HttpResponsePtr &response)
{
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
}

int parsePriceParameter(const std::string &value, int defaultValue)
{
    if (value.empty()) {
        return defaultValue;
    }

    try {
        int parsed = std::stoi(value);

        if (parsed < 0) {
            return defaultValue;
        }

        return parsed;
    } catch (...) {
        return defaultValue;
    }
}

std::vector<std::string> getImagesFromJson(const Json::Value &json)
{
    std::vector<std::string> images;

    if (json.isMember("images") && json["images"].isArray()) {
        for (Json::ArrayIndex i = 0; i < json["images"].size(); ++i) {
            const std::string url = json["images"][i].asString();

            if (!url.empty()) {
                images.push_back(url);
            }
        }
    }

    if (images.empty() && json.isMember("image")) {
        const std::string image = json["image"].asString();

        if (!image.empty()) {
            images.push_back(image);
        }
    }

    if (images.size() < 2 && json.isMember("hoverImage")) {
        const std::string hoverImage = json["hoverImage"].asString();

        if (!hoverImage.empty()) {
            images.push_back(hoverImage);
        }
    }

    return images;
}

std::string generateResetToken()
{
    static const char hex[] = "0123456789abcdef";

    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    std::uniform_int_distribution<int> distribution(0, 255);

    std::string token;
    token.reserve(64);

    for (int i = 0; i < 32; ++i) {
        const int byte = distribution(generator);

        token.push_back(hex[(byte >> 4) & 0x0F]);
        token.push_back(hex[byte & 0x0F]);
    }

    return token;
}

std::string getProductsOrderBy(const std::string &sort)
{
    if (sort == "Сначала дешевле") {
        return "p.price ASC, p.id ASC";
    }

    if (sort == "Сначала дороже") {
        return "p.price DESC, p.id ASC";
    }

    if (sort == "По названию") {
        return "p.title ASC, p.id ASC";
    }

    return "p.id ASC";
}

int main()
{
    auto dbClient = drogon::orm::DbClient::newPgClient(
        "host=127.0.0.1 port=5432 dbname=pixio_db user=pixio_user password=pixio_password",
        1
    );

    app().registerHandler(
        "/api/health",
        [](const HttpRequestPtr &req,
           std::function<void(const HttpResponsePtr &)> &&callback)
        {
            Json::Value result;
            result["status"] = "ok";
            result["message"] = "PIXIO backend is running";

            auto response = HttpResponse::newHttpJsonResponse(result);
            addCorsHeaders(response);

            callback(response);
        },
        {Get}
    );

    app().registerHandler(
        "/api/auth/login",
        [](const HttpRequestPtr &req,
           std::function<void(const HttpResponsePtr &)> &&callback)
        {
            auto response = HttpResponse::newHttpResponse();
            response->setStatusCode(k204NoContent);
            addCorsHeaders(response);
            callback(response);
        },
        {Options}
    );

    app().registerHandler(
        "/api/auth/login",
        [dbClient](const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback)
        {
            auto json = req->getJsonObject();

            if (!json || !json->isMember("email") || !json->isMember("password")) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Email and password are required";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            const std::string email = (*json)["email"].asString();
            const std::string password = (*json)["password"].asString();

            dbClient->execSqlAsync(
                "SELECT id, name, email, role "
                "FROM users "
                "WHERE lower(email) = lower($1) "
                "AND password_hash = crypt($2, password_hash) "
                "LIMIT 1",
                [callback](const drogon::orm::Result &result)
                {
                    if (result.empty()) {
                        Json::Value error;
                        error["success"] = false;
                        error["message"] = "Invalid email or password";

                        auto response = HttpResponse::newHttpJsonResponse(error);
                        response->setStatusCode(k401Unauthorized);
                        addCorsHeaders(response);

                        callback(response);
                        return;
                    }

                    const auto row = result[0];

                    Json::Value user;
                    user["id"] = row["id"].as<int>();
                    user["name"] = row["name"].as<std::string>();
                    user["email"] = row["email"].as<std::string>();
                    user["role"] = row["role"].as<std::string>();

                    Json::Value body;
                    body["success"] = true;
                    body["user"] = user;

                    auto response = HttpResponse::newHttpJsonResponse(body);
                    addCorsHeaders(response);

                    callback(response);
                },
                [callback](const drogon::orm::DrogonDbException &e)
                {
                    Json::Value error;
                    error["success"] = false;
                    error["message"] = "Database error";

                    auto response = HttpResponse::newHttpJsonResponse(error);
                    response->setStatusCode(k500InternalServerError);
                    addCorsHeaders(response);

                    callback(response);
                },
                email,
                password
            );
        },
        {Post}
    );

    app().registerHandler(
        "/api/auth/register",
        [](const HttpRequestPtr &req,
           std::function<void(const HttpResponsePtr &)> &&callback)
        {
            auto response = HttpResponse::newHttpResponse();
            response->setStatusCode(k204NoContent);
            addCorsHeaders(response);
            callback(response);
        },
        {Options}
    );

    app().registerHandler(
        "/api/auth/register",
        [dbClient](const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback)
        {
            auto json = req->getJsonObject();

            if (
                !json ||
                !json->isMember("name") ||
                !json->isMember("email") ||
                !json->isMember("password")
            ) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Name, email and password are required";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            const std::string name = (*json)["name"].asString();
            const std::string email = (*json)["email"].asString();
            const std::string password = (*json)["password"].asString();

            if (name.empty() || email.empty() || password.empty()) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Fields cannot be empty";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            if (password.length() < 6) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Password must contain at least 6 characters";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            dbClient->execSqlAsync(
                "INSERT INTO users (name, email, password_hash, role) "
                "VALUES ($1, lower($2), crypt($3, gen_salt('bf')), 'buyer') "
                "ON CONFLICT (email) DO NOTHING "
                "RETURNING id, name, email, role",
                [callback](const drogon::orm::Result &result)
                {
                    if (result.empty()) {
                        Json::Value error;
                        error["success"] = false;
                        error["message"] = "User with this email already exists";

                        auto response = HttpResponse::newHttpJsonResponse(error);
                        response->setStatusCode(k409Conflict);
                        addCorsHeaders(response);

                        callback(response);
                        return;
                    }

                    const auto row = result[0];

                    Json::Value user;
                    user["id"] = row["id"].as<int>();
                    user["name"] = row["name"].as<std::string>();
                    user["email"] = row["email"].as<std::string>();
                    user["role"] = row["role"].as<std::string>();

                    Json::Value body;
                    body["success"] = true;
                    body["user"] = user;

                    auto response = HttpResponse::newHttpJsonResponse(body);
                    response->setStatusCode(k201Created);
                    addCorsHeaders(response);

                    callback(response);
                },
                [callback](const drogon::orm::DrogonDbException &e)
                {
                    Json::Value error;
                    error["success"] = false;
                    error["message"] = "Database error";

                    auto response = HttpResponse::newHttpJsonResponse(error);
                    response->setStatusCode(k500InternalServerError);
                    addCorsHeaders(response);

                    callback(response);
                },
                name,
                email,
                password
            );
        },
        {Post}
    );

    app().registerHandler(
        "/api/auth/forgot-password",
        [](const HttpRequestPtr &req,
           std::function<void(const HttpResponsePtr &)> &&callback)
        {
            auto response = HttpResponse::newHttpResponse();
            response->setStatusCode(k204NoContent);
            addCorsHeaders(response);
            callback(response);
        },
        {Options}
    );

    app().registerHandler(
        "/api/auth/forgot-password",
        [dbClient](const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback)
        {
            auto json = req->getJsonObject();

            if (!json || !json->isMember("email")) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Email is required";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            const std::string email = (*json)["email"].asString();

            if (email.empty()) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Email cannot be empty";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            const std::string token = generateResetToken();
            const std::string resetLink =
                "http://localhost:4321/reset-password?token=" + token;

            dbClient->execSqlAsync(
                "WITH target_user AS ( "
                "    SELECT id "
                "    FROM users "
                "    WHERE lower(email) = lower($1) "
                "    LIMIT 1 "
                "), "
                "disabled_old_tokens AS ( "
                "    UPDATE password_reset_tokens "
                "    SET used = true "
                "    WHERE user_id IN (SELECT id FROM target_user) "
                "    AND used = false "
                "), "
                "created_token AS ( "
                "    INSERT INTO password_reset_tokens (user_id, token_hash, expires_at) "
                "    SELECT id, crypt($2, gen_salt('bf')), now() + interval '30 minutes' "
                "    FROM target_user "
                "    RETURNING id "
                ") "
                "SELECT EXISTS(SELECT 1 FROM target_user) AS user_exists",
                [callback, resetLink](const drogon::orm::Result &result)
                {
                    const bool userExists = result[0]["user_exists"].as<bool>();

                    Json::Value body;
                    body["success"] = true;
                    body["message"] = "If account exists, reset instructions were sent";

                    if (userExists) {
                        body["resetLink"] = resetLink;
                    } else {
                        body["resetLink"] = "";
                    }

                    auto response = HttpResponse::newHttpJsonResponse(body);
                    addCorsHeaders(response);

                    callback(response);
                },
                [callback](const drogon::orm::DrogonDbException &e)
                {
                    Json::Value error;
                    error["success"] = false;
                    error["message"] = "Database error";

                    auto response = HttpResponse::newHttpJsonResponse(error);
                    response->setStatusCode(k500InternalServerError);
                    addCorsHeaders(response);

                    callback(response);
                },
                email,
                token
            );
        },
        {Post}
    );

    app().registerHandler(
        "/api/auth/reset-password",
        [](const HttpRequestPtr &req,
           std::function<void(const HttpResponsePtr &)> &&callback)
        {
            auto response = HttpResponse::newHttpResponse();
            response->setStatusCode(k204NoContent);
            addCorsHeaders(response);
            callback(response);
        },
        {Options}
    );

    app().registerHandler(
        "/api/auth/reset-password",
        [dbClient](const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback)
        {
            auto json = req->getJsonObject();

            if (
                !json ||
                !json->isMember("token") ||
                !json->isMember("password")
            ) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Token and password are required";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            const std::string token = (*json)["token"].asString();
            const std::string password = (*json)["password"].asString();

            if (token.empty() || password.empty()) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Token and password cannot be empty";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            if (password.length() < 6) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Password must contain at least 6 characters";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            dbClient->execSqlAsync(
                "WITH token_row AS ( "
                "    SELECT id, user_id "
                "    FROM password_reset_tokens "
                "    WHERE used = false "
                "    AND expires_at > now() "
                "    AND token_hash = crypt($1, token_hash) "
                "    ORDER BY created_at DESC "
                "    LIMIT 1 "
                "), "
                "updated_user AS ( "
                "    UPDATE users "
                "    SET password_hash = crypt($2, gen_salt('bf')) "
                "    WHERE id IN (SELECT user_id FROM token_row) "
                "    RETURNING id "
                "), "
                "updated_token AS ( "
                "    UPDATE password_reset_tokens "
                "    SET used = true "
                "    WHERE id IN (SELECT id FROM token_row) "
                "    RETURNING id "
                ") "
                "SELECT EXISTS(SELECT 1 FROM updated_user) AS password_changed",
                [callback](const drogon::orm::Result &result)
                {
                    const bool passwordChanged =
                        result[0]["password_changed"].as<bool>();

                    if (!passwordChanged) {
                        Json::Value error;
                        error["success"] = false;
                        error["message"] = "Invalid or expired reset token";

                        auto response = HttpResponse::newHttpJsonResponse(error);
                        response->setStatusCode(k400BadRequest);
                        addCorsHeaders(response);

                        callback(response);
                        return;
                    }

                    Json::Value body;
                    body["success"] = true;
                    body["message"] = "Password has been changed";

                    auto response = HttpResponse::newHttpJsonResponse(body);
                    addCorsHeaders(response);

                    callback(response);
                },
                [callback](const drogon::orm::DrogonDbException &e)
                {
                    Json::Value error;
                    error["success"] = false;
                    error["message"] = "Database error";

                    auto response = HttpResponse::newHttpJsonResponse(error);
                    response->setStatusCode(k500InternalServerError);
                    addCorsHeaders(response);

                    callback(response);
                },
                token,
                password
            );
        },
        {Post}
    );

    app().registerHandler(
        "/api/products",
        [](const HttpRequestPtr &req,
           std::function<void(const HttpResponsePtr &)> &&callback)
        {
            auto response = HttpResponse::newHttpResponse();
            response->setStatusCode(k204NoContent);
            addCorsHeaders(response);
            callback(response);
        },
        {Options}
    );

    app().registerHandler(
        "/api/products",
        [dbClient](const HttpRequestPtr &req,
                std::function<void(const HttpResponsePtr &)> &&callback)
        {
            const std::string search = req->getParameter("search");
            const std::string category = req->getParameter("category");
            const std::string sort = req->getParameter("sort");

            const int minPrice = parsePriceParameter(
                req->getParameter("minPrice"),
                0
            );

            const int maxPrice = parsePriceParameter(
                req->getParameter("maxPrice"),
                std::numeric_limits<int>::max()
            );

            const std::string orderBy = getProductsOrderBy(sort);

            const std::string sql =
                "SELECT "
                "p.id, "
                "p.seller_id, "
                "p.title, "
                "p.category, "
                "p.description, "
                "p.price, "
                "p.image, "
                "COALESCE(p.hover_image, '') AS hover_image, "
                "pi.url AS image_url "
                "FROM products p "
                "LEFT JOIN product_images pi ON pi.product_id = p.id "
                "WHERE "
                "($1 = '' OR p.title ILIKE '%' || $1 || '%' OR p.description ILIKE '%' || $1 || '%') "
                "AND ($2 = '' OR p.category = $2) "
                "AND p.price >= $3 "
                "AND p.price <= $4 "
                "ORDER BY " + orderBy + ", pi.sort_order ASC, pi.id ASC";

            dbClient->execSqlAsync(
                sql,
                [callback](const drogon::orm::Result &result)
                {
                    Json::Value productsById;
                    Json::Value productOrder(Json::arrayValue);

                    for (const auto &row : result) {
                        const int productId = row["id"].as<int>();
                        const std::string key = std::to_string(productId);

                        if (!productsById.isMember(key)) {
                            Json::Value product;

                            product["id"] = productId;
                            product["sellerId"] = row["seller_id"].as<int>();
                            product["title"] = row["title"].as<std::string>();
                            product["category"] = row["category"].as<std::string>();
                            product["description"] = row["description"].as<std::string>();
                            product["price"] = row["price"].as<int>();
                            product["image"] = row["image"].as<std::string>();
                            product["hoverImage"] = row["hover_image"].as<std::string>();
                            product["images"] = Json::arrayValue;

                            productsById[key] = product;
                            productOrder.append(productId);
                        }

                        if (!row["image_url"].isNull()) {
                            productsById[key]["images"].append(
                                row["image_url"].as<std::string>()
                            );
                        }
                    }

                    Json::Value products(Json::arrayValue);

                    for (Json::ArrayIndex i = 0; i < productOrder.size(); ++i) {
                        const std::string key = std::to_string(productOrder[i].asInt());

                        if (productsById[key]["images"].size() == 0) {
                            const std::string image = productsById[key]["image"].asString();
                            const std::string hoverImage = productsById[key]["hoverImage"].asString();

                            if (!image.empty()) {
                                productsById[key]["images"].append(image);
                            }

                            if (!hoverImage.empty()) {
                                productsById[key]["images"].append(hoverImage);
                            }
                        }

                        products.append(productsById[key]);
                    }

                    Json::Value body;
                    body["success"] = true;
                    body["products"] = products;

                    auto response = HttpResponse::newHttpJsonResponse(body);
                    addCorsHeaders(response);

                    callback(response);
                },
                [callback](const drogon::orm::DrogonDbException &e)
                {
                    Json::Value error;
                    error["success"] = false;
                    error["message"] = "Failed to load products";

                    auto response = HttpResponse::newHttpJsonResponse(error);
                    response->setStatusCode(k500InternalServerError);
                    addCorsHeaders(response);

                    callback(response);
                },
                search,
                category,
                minPrice,
                maxPrice
            );
        },
        {Get}
    );

    app().registerHandler(
        "/api/products",
        [dbClient](const HttpRequestPtr &req,
                std::function<void(const HttpResponsePtr &)> &&callback)
        {
            auto json = req->getJsonObject();

            if (
                !json ||
                !json->isMember("sellerId") ||
                !json->isMember("title") ||
                !json->isMember("category") ||
                !json->isMember("description") ||
                !json->isMember("price")
            ) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Required product fields are missing";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            const int sellerId = (*json)["sellerId"].asInt();
            const std::string title = (*json)["title"].asString();
            const std::string category = (*json)["category"].asString();
            const std::string description = (*json)["description"].asString();
            const int price = (*json)["price"].asInt();

            const std::vector<std::string> images = getImagesFromJson(*json);

            const std::string image = images.size() > 0 ? images[0] : "";
            const std::string hoverImage = images.size() > 1 ? images[1] : "";

            if (
                sellerId <= 0 ||
                title.empty() ||
                category.empty() ||
                description.empty() ||
                price < 0 ||
                image.empty()
            ) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Product fields are invalid";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            try {
                auto result = dbClient->execSqlSync(
                    "INSERT INTO products "
                    "(seller_id, title, category, description, price, image, hover_image) "
                    "VALUES ($1, $2, $3, $4, $5, $6, $7) "
                    "RETURNING id, seller_id, title, category, description, price, image, hover_image",
                    sellerId,
                    title,
                    category,
                    description,
                    price,
                    image,
                    hoverImage
                );

                const auto row = result[0];
                const int productId = row["id"].as<int>();

                for (size_t i = 0; i < images.size(); ++i) {
                    dbClient->execSqlSync(
                        "INSERT INTO product_images (product_id, url, sort_order) "
                        "VALUES ($1, $2, $3)",
                        productId,
                        images[i],
                        static_cast<int>(i)
                    );
                }

                Json::Value product;
                product["id"] = productId;
                product["sellerId"] = row["seller_id"].as<int>();
                product["title"] = row["title"].as<std::string>();
                product["category"] = row["category"].as<std::string>();
                product["description"] = row["description"].as<std::string>();
                product["price"] = row["price"].as<int>();
                product["image"] = row["image"].as<std::string>();

                if (row["hover_image"].isNull()) {
                    product["hoverImage"] = "";
                } else {
                    product["hoverImage"] = row["hover_image"].as<std::string>();
                }

                Json::Value imagesJson(Json::arrayValue);

                for (const auto &url : images) {
                    imagesJson.append(url);
                }

                product["images"] = imagesJson;

                Json::Value body;
                body["success"] = true;
                body["product"] = product;

                auto response = HttpResponse::newHttpJsonResponse(body);
                response->setStatusCode(k201Created);
                addCorsHeaders(response);

                callback(response);
            } catch (const drogon::orm::DrogonDbException &e) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Database error";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k500InternalServerError);
                addCorsHeaders(response);

                callback(response);
            }
        },
        {Post}
    );

    app().registerHandler(
        "/api/products/{1}",
        [dbClient](const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback,
                   int productId)
        {
            if (req->method() == Options) {
                auto response = HttpResponse::newHttpResponse();
                response->setStatusCode(k204NoContent);
                addCorsHeaders(response);
                callback(response);
                return;
            }

            if (productId <= 0) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Invalid product id";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            if (req->method() == Delete) {
                dbClient->execSqlAsync(
                    "DELETE FROM products "
                    "WHERE id = $1 "
                    "RETURNING id",
                    [callback](const drogon::orm::Result &result)
                    {
                        if (result.empty()) {
                            Json::Value error;
                            error["success"] = false;
                            error["message"] = "Product not found";

                            auto response = HttpResponse::newHttpJsonResponse(error);
                            response->setStatusCode(k404NotFound);
                            addCorsHeaders(response);

                            callback(response);
                            return;
                        }

                        Json::Value body;
                        body["success"] = true;
                        body["message"] = "Product deleted";

                        auto response = HttpResponse::newHttpJsonResponse(body);
                        addCorsHeaders(response);

                        callback(response);
                    },
                    [callback](const drogon::orm::DrogonDbException &e)
                    {
                        Json::Value error;
                        error["success"] = false;
                        error["message"] = "Database error";

                        auto response = HttpResponse::newHttpJsonResponse(error);
                        response->setStatusCode(k500InternalServerError);
                        addCorsHeaders(response);

                        callback(response);
                    },
                    productId
                );

                return;
            }

            if (req->method() == Put) {
                auto json = req->getJsonObject();

                if (
                    !json ||
                    !json->isMember("title") ||
                    !json->isMember("category") ||
                    !json->isMember("description") ||
                    !json->isMember("price")
                ) {
                    Json::Value error;
                    error["success"] = false;
                    error["message"] = "Required product fields are missing";

                    auto response = HttpResponse::newHttpJsonResponse(error);
                    response->setStatusCode(k400BadRequest);
                    addCorsHeaders(response);

                    callback(response);
                    return;
                }

                const std::string title = (*json)["title"].asString();
                const std::string category = (*json)["category"].asString();
                const std::string description = (*json)["description"].asString();
                const int price = (*json)["price"].asInt();

                const std::vector<std::string> images = getImagesFromJson(*json);

                const std::string image = images.size() > 0 ? images[0] : "";
                const std::string hoverImage = images.size() > 1 ? images[1] : "";

                if (
                    title.empty() ||
                    category.empty() ||
                    description.empty() ||
                    price < 0 ||
                    image.empty()
                ) {
                    Json::Value error;
                    error["success"] = false;
                    error["message"] = "Product fields are invalid";

                    auto response = HttpResponse::newHttpJsonResponse(error);
                    response->setStatusCode(k400BadRequest);
                    addCorsHeaders(response);

                    callback(response);
                    return;
                }

                try {
                    auto result = dbClient->execSqlSync(
                        "UPDATE products "
                        "SET title = $1, "
                        "category = $2, "
                        "description = $3, "
                        "price = $4, "
                        "image = $5, "
                        "hover_image = $6 "
                        "WHERE id = $7 "
                        "RETURNING id, seller_id, title, category, description, price, image, hover_image",
                        title,
                        category,
                        description,
                        price,
                        image,
                        hoverImage,
                        productId
                    );

                    if (result.empty()) {
                        Json::Value error;
                        error["success"] = false;
                        error["message"] = "Product not found";

                        auto response = HttpResponse::newHttpJsonResponse(error);
                        response->setStatusCode(k404NotFound);
                        addCorsHeaders(response);

                        callback(response);
                        return;
                    }

                    dbClient->execSqlSync(
                        "DELETE FROM product_images WHERE product_id = $1",
                        productId
                    );

                    for (size_t i = 0; i < images.size(); ++i) {
                        dbClient->execSqlSync(
                            "INSERT INTO product_images (product_id, url, sort_order) "
                            "VALUES ($1, $2, $3)",
                            productId,
                            images[i],
                            static_cast<int>(i)
                        );
                    }

                    const auto row = result[0];

                    Json::Value product;
                    product["id"] = row["id"].as<int>();
                    product["sellerId"] = row["seller_id"].as<int>();
                    product["title"] = row["title"].as<std::string>();
                    product["category"] = row["category"].as<std::string>();
                    product["description"] = row["description"].as<std::string>();
                    product["price"] = row["price"].as<int>();
                    product["image"] = row["image"].as<std::string>();

                    if (row["hover_image"].isNull()) {
                        product["hoverImage"] = "";
                    } else {
                        product["hoverImage"] = row["hover_image"].as<std::string>();
                    }

                    Json::Value imagesJson(Json::arrayValue);

                    for (const auto &url : images) {
                        imagesJson.append(url);
                    }

                    product["images"] = imagesJson;

                    Json::Value body;
                    body["success"] = true;
                    body["product"] = product;

                    auto response = HttpResponse::newHttpJsonResponse(body);
                    addCorsHeaders(response);

                    callback(response);
                } catch (const drogon::orm::DrogonDbException &e) {
                    Json::Value error;
                    error["success"] = false;
                    error["message"] = "Database error";

                    auto response = HttpResponse::newHttpJsonResponse(error);
                    response->setStatusCode(k500InternalServerError);
                    addCorsHeaders(response);

                    callback(response);
                }

                return;
            }

            Json::Value error;
            error["success"] = false;
            error["message"] = "Method not allowed";

            auto response = HttpResponse::newHttpJsonResponse(error);
            response->setStatusCode(k405MethodNotAllowed);
            addCorsHeaders(response);

            callback(response);
        },
        {Put, Delete, Options}
    );

    app().registerHandler(
        "/api/orders",
        [](const HttpRequestPtr &req,
           std::function<void(const HttpResponsePtr &)> &&callback)
        {
            auto response = HttpResponse::newHttpResponse();
            response->setStatusCode(k204NoContent);
            addCorsHeaders(response);
            callback(response);
        },
        {Options}
    );

    app().registerHandler(
        "/api/orders",
        [dbClient](const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback)
        {
            dbClient->execSqlAsync(
                "SELECT "
                "o.id AS order_id, "
                "o.user_id, "
                "o.total, "
                "o.status, "
                "o.created_at, "
                "oi.product_id, "
                "oi.title, "
                "oi.price, "
                "oi.category, "
                "oi.image, "
                "oi.license_key "
                "FROM orders o "
                "LEFT JOIN order_items oi ON oi.order_id = o.id "
                "ORDER BY o.created_at DESC, o.id DESC, oi.id ASC",
                [callback](const drogon::orm::Result &result)
                {
                    Json::Value ordersMap;

                    for (const auto &row : result) {
                        const int orderId = row["order_id"].as<int>();
                        const std::string key = std::to_string(orderId);

                        if (!ordersMap.isMember(key)) {
                            Json::Value order;
                            order["id"] = orderId;
                            order["userId"] = row["user_id"].as<int>();
                            order["total"] = row["total"].as<int>();
                            order["status"] = row["status"].as<std::string>();
                            order["date"] = row["created_at"].as<std::string>();
                            order["items"] = Json::arrayValue;

                            ordersMap[key] = order;
                        }

                        if (!row["product_id"].isNull()) {
                            Json::Value item;
                            item["productId"] = row["product_id"].as<int>();
                            item["title"] = row["title"].as<std::string>();
                            item["price"] = row["price"].as<int>();
                            item["category"] = row["category"].as<std::string>();
                            item["image"] = row["image"].as<std::string>();
                            item["licenseKey"] = row["license_key"].as<std::string>();

                            ordersMap[key]["items"].append(item);
                        }
                    }

                    Json::Value orders = Json::arrayValue;

                    for (const auto &key : ordersMap.getMemberNames()) {
                        orders.append(ordersMap[key]);
                    }

                    Json::Value body;
                    body["success"] = true;
                    body["orders"] = orders;

                    auto response = HttpResponse::newHttpJsonResponse(body);
                    addCorsHeaders(response);

                    callback(response);
                },
                [callback](const drogon::orm::DrogonDbException &e)
                {
                    Json::Value error;
                    error["success"] = false;
                    error["message"] = "Database error";

                    auto response = HttpResponse::newHttpJsonResponse(error);
                    response->setStatusCode(k500InternalServerError);
                    addCorsHeaders(response);

                    callback(response);
                }
            );
        },
        {Get}
    );

    app().registerHandler(
        "/api/orders",
        [dbClient](const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback)
        {
            auto json = req->getJsonObject();

            if (
                !json ||
                !json->isMember("userId") ||
                !json->isMember("items") ||
                !(*json)["items"].isArray() ||
                (*json)["items"].size() == 0
            ) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Order fields are missing";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            const int userId = (*json)["userId"].asInt();
            const Json::Value items = (*json)["items"];

            if (userId <= 0) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Invalid user id";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            int total = 0;

            for (Json::ArrayIndex i = 0; i < items.size(); ++i) {
                const Json::Value item = items[i];

                if (
                    !item.isMember("productId") ||
                    !item.isMember("title") ||
                    !item.isMember("price") ||
                    !item.isMember("category") ||
                    !item.isMember("image")
                ) {
                    Json::Value error;
                    error["success"] = false;
                    error["message"] = "Order item fields are missing";

                    auto response = HttpResponse::newHttpJsonResponse(error);
                    response->setStatusCode(k400BadRequest);
                    addCorsHeaders(response);

                    callback(response);
                    return;
                }

                const int productId = item["productId"].asInt();
                const int price = item["price"].asInt();
                const std::string title = item["title"].asString();
                const std::string category = item["category"].asString();
                const std::string image = item["image"].asString();

                if (
                    productId <= 0 ||
                    price < 0 ||
                    title.empty() ||
                    category.empty() ||
                    image.empty()
                ) {
                    Json::Value error;
                    error["success"] = false;
                    error["message"] = "Order item fields are invalid";

                    auto response = HttpResponse::newHttpJsonResponse(error);
                    response->setStatusCode(k400BadRequest);
                    addCorsHeaders(response);

                    callback(response);
                    return;
                }

                total += price;
            }

            try {
                auto orderResult = dbClient->execSqlSync(
                    "INSERT INTO orders (user_id, total, status) "
                    "VALUES ($1, $2, 'Оплачен') "
                    "RETURNING id, user_id, total, status, created_at",
                    userId,
                    total
                );

                const auto orderRow = orderResult[0];
                const int orderId = orderRow["id"].as<int>();

                Json::Value orderItems = Json::arrayValue;

                for (Json::ArrayIndex i = 0; i < items.size(); ++i) {
                    const Json::Value item = items[i];

                    const int productId = item["productId"].asInt();
                    const std::string title = item["title"].asString();
                    const int price = item["price"].asInt();
                    const std::string category = item["category"].asString();
                    const std::string image = item["image"].asString();

                    std::string licenseKey = "";

                    if (item.isMember("licenseKey")) {
                        licenseKey = item["licenseKey"].asString();
                    }

                    if (licenseKey.empty()) {
                        licenseKey =
                            "PX-" +
                            std::to_string(orderId) +
                            "-" +
                            std::to_string(productId) +
                            "-" +
                            std::to_string(i + 1);
                    }

                    dbClient->execSqlSync(
                        "INSERT INTO order_items "
                        "(order_id, product_id, title, price, category, image, license_key) "
                        "VALUES ($1, $2, $3, $4, $5, $6, $7)",
                        orderId,
                        productId,
                        title,
                        price,
                        category,
                        image,
                        licenseKey
                    );

                    Json::Value orderItem;
                    orderItem["productId"] = productId;
                    orderItem["title"] = title;
                    orderItem["price"] = price;
                    orderItem["category"] = category;
                    orderItem["image"] = image;
                    orderItem["licenseKey"] = licenseKey;

                    orderItems.append(orderItem);
                }

                Json::Value order;
                order["id"] = orderId;
                order["userId"] = orderRow["user_id"].as<int>();
                order["total"] = orderRow["total"].as<int>();
                order["status"] = orderRow["status"].as<std::string>();
                order["date"] = orderRow["created_at"].as<std::string>();
                order["items"] = orderItems;

                Json::Value body;
                body["success"] = true;
                body["order"] = order;

                auto response = HttpResponse::newHttpJsonResponse(body);
                response->setStatusCode(k201Created);
                addCorsHeaders(response);

                callback(response);
            } catch (const drogon::orm::DrogonDbException &e) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Database error";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k500InternalServerError);
                addCorsHeaders(response);

                callback(response);
            }
        },
        {Post}
    );

    app().registerHandler(
        "/api/orders/{1}/status",
        [dbClient](const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback,
                   int orderId)
        {
            if (req->method() == Options) {
                auto response = HttpResponse::newHttpResponse();
                response->setStatusCode(k204NoContent);
                addCorsHeaders(response);
                callback(response);
                return;
            }

            if (orderId <= 0) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Invalid order id";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            auto json = req->getJsonObject();

            if (!json || !json->isMember("status")) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Status is required";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            const std::string status = (*json)["status"].asString();

            if (status != "Оплачен" && status != "Выполнен") {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Invalid status";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            dbClient->execSqlAsync(
                "UPDATE orders "
                "SET status = $1 "
                "WHERE id = $2 "
                "RETURNING id, user_id, total, status, created_at",
                [callback](const drogon::orm::Result &result)
                {
                    if (result.empty()) {
                        Json::Value error;
                        error["success"] = false;
                        error["message"] = "Order not found";

                        auto response = HttpResponse::newHttpJsonResponse(error);
                        response->setStatusCode(k404NotFound);
                        addCorsHeaders(response);

                        callback(response);
                        return;
                    }

                    const auto row = result[0];

                    Json::Value order;
                    order["id"] = row["id"].as<int>();
                    order["userId"] = row["user_id"].as<int>();
                    order["total"] = row["total"].as<int>();
                    order["status"] = row["status"].as<std::string>();
                    order["date"] = row["created_at"].as<std::string>();

                    Json::Value body;
                    body["success"] = true;
                    body["order"] = order;

                    auto response = HttpResponse::newHttpJsonResponse(body);
                    addCorsHeaders(response);

                    callback(response);
                },
                [callback](const drogon::orm::DrogonDbException &e)
                {
                    Json::Value error;
                    error["success"] = false;
                    error["message"] = "Database error";

                    auto response = HttpResponse::newHttpJsonResponse(error);
                    response->setStatusCode(k500InternalServerError);
                    addCorsHeaders(response);

                    callback(response);
                },
                status,
                orderId
            );
        },
        {Put, Options}
    );

    app().registerHandler(
        "/api/users/{1}",
        [dbClient](const HttpRequestPtr &req,
                   std::function<void(const HttpResponsePtr &)> &&callback,
                   int userId)
        {
            if (req->method() == Options) {
                auto response = HttpResponse::newHttpResponse();
                response->setStatusCode(k204NoContent);
                addCorsHeaders(response);
                callback(response);
                return;
            }

            if (userId <= 0) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Invalid user id";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            auto json = req->getJsonObject();

            if (!json || !json->isMember("name") || !json->isMember("email")) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Name and email are required";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            const std::string name = (*json)["name"].asString();
            const std::string email = (*json)["email"].asString();

            if (name.empty() || email.empty()) {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Name and email cannot be empty";

                auto response = HttpResponse::newHttpJsonResponse(error);
                response->setStatusCode(k400BadRequest);
                addCorsHeaders(response);

                callback(response);
                return;
            }

            dbClient->execSqlAsync(
                "UPDATE users "
                "SET name = $1, email = $2 "
                "WHERE id = $3 "
                "RETURNING id, name, email, role",
                [callback](const drogon::orm::Result &result)
                {
                    if (result.empty()) {
                        Json::Value error;
                        error["success"] = false;
                        error["message"] = "User not found";

                        auto response = HttpResponse::newHttpJsonResponse(error);
                        response->setStatusCode(k404NotFound);
                        addCorsHeaders(response);

                        callback(response);
                        return;
                    }

                    const auto row = result[0];

                    Json::Value user;
                    user["id"] = row["id"].as<int>();
                    user["name"] = row["name"].as<std::string>();
                    user["email"] = row["email"].as<std::string>();
                    user["role"] = row["role"].as<std::string>();

                    Json::Value body;
                    body["success"] = true;
                    body["user"] = user;

                    auto response = HttpResponse::newHttpJsonResponse(body);
                    addCorsHeaders(response);

                    callback(response);
                },
                [callback](const drogon::orm::DrogonDbException &e)
                {
                    Json::Value error;
                    error["success"] = false;
                    error["message"] = "Email already exists or database error";

                    auto response = HttpResponse::newHttpJsonResponse(error);
                    response->setStatusCode(k409Conflict);
                    addCorsHeaders(response);

                    callback(response);
                },
                name,
                email,
                userId
            );
        },
        {Put, Options}
    );

    app()
        .addListener("0.0.0.0", 5555)
        .run();

    return 0;
}