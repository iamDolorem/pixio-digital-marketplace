users
- id
- name
- email
- password_hash
- role

products
- id
- seller_id
- title
- category
- description
- price
- image
- hover_image
- created_at

orders
- id
- user_id
- total
- status
- created_at

order_items
- id
- order_id
- product_id
- title
- price
- license_key



POST /auth/register
POST /auth/login
POST /auth/logout
GET  /auth/me

GET  /products
GET  /products/:id
POST /products
PUT  /products/:id
DELETE /products/:id

GET  /cart
POST /cart/items
DELETE /cart/items/:productId
DELETE /cart

POST /orders
GET  /orders/my
PATCH /orders/:id/status