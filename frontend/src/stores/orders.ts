import { writable, get } from 'svelte/store';
import type { CartItem } from './cart';

export type OrderItem = CartItem & {
  licenseKey: string;
};

export type Order = {
  id: number;
  userId: number;
  date: string;
  status: 'Оплачен' | 'Выполнен';
  items: OrderItem[];
  total: number;
};

type BackendOrderItem = {
  productId: number;
  title: string;
  price: number;
  category: string;
  image: string;
  licenseKey: string;
};

type BackendOrder = {
  id: number;
  userId: number;
  date: string;
  status: 'Оплачен' | 'Выполнен';
  items: BackendOrderItem[];
  total: number;
};

const API_URL = 'http://192.168.1.87:5555/api/orders';

const generateLicenseKey = () => {
  const part = () =>
    Math.random().toString(36).substring(2, 6).toUpperCase();

  return `PX-${part()}-${part()}-${part()}`;
};

function createOrdersStore() {
  const store = writable<Order[]>([]);

  const normalizeOrder = (order: BackendOrder): Order => {
    return {
      id: Number(order.id),
      userId: Number(order.userId),
      date: String(order.date),
      status: order.status,
      total: Number(order.total),
      items: Array.isArray(order.items)
        ? order.items.map((item) => ({
            productId: Number(item.productId),
            title: String(item.title),
            price: Number(item.price),
            category: String(item.category),
            image: String(item.image || ''),
            licenseKey: String(item.licenseKey || generateLicenseKey())
          }))
        : []
    };
  };

  const load = async () => {
    try {
      const response = await fetch(API_URL);

      if (!response.ok) {
        console.error('Failed to load orders from backend');
        return;
      }

      const data = await response.json();

      if (!data.success || !Array.isArray(data.orders)) {
        console.error('Invalid orders response', data);
        return;
      }

      const orders: Order[] = data.orders
        .map((order: BackendOrder) => normalizeOrder(order))
        .sort((a: Order, b: Order) => b.id - a.id);

      store.set(orders);
    } catch (error) {
      console.error('Failed to load orders from backend', error);
    }
  };

  const createOrder = async (userId: number, items: CartItem[]) => {
    try {
      const response = await fetch(API_URL, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({
          userId,
          items: items.map((item) => ({
            productId: item.productId,
            title: item.title,
            price: item.price,
            category: item.category,
            image: item.image
          }))
        })
      });

      const data = await response.json();

      if (!response.ok || !data.success || !data.order) {
        console.error('Failed to create order', data);
        return null;
      }

      const newOrder = normalizeOrder(data.order);

      store.set([newOrder, ...get(store)]);

      return newOrder;
    } catch (error) {
      console.error('Failed to create order through backend', error);
      return null;
    }
  };

  const getOrdersByUserId = (userId: number) => {
    return get(store).filter((order) => order.userId === userId);
  };

  const updateStatus = async (orderId: number, status: Order['status']) => {
    try {
      const response = await fetch(`${API_URL}/${orderId}/status`, {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({ status })
      });

      const data = await response.json();

      if (!response.ok || !data.success) {
        console.error('Failed to update order status', data);
        return false;
      }

      const orders = get(store);

      const updated = orders.map((order) =>
        order.id === orderId ? { ...order, status } : order
      );

      store.set(updated);

      return true;
    } catch (error) {
      console.error('Failed to update order status through backend', error);
      return false;
    }
  };

  return {
    subscribe: store.subscribe,
    load,
    createOrder,
    getOrdersByUserId,
    updateStatus
  };
}

export const ordersStore = createOrdersStore();