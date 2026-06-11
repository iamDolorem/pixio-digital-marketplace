import { writable, get } from 'svelte/store';

export type CartItem = {
  productId: number;
  title: string;
  price: number;
  category: string;
  image: string;
};

const STORAGE_KEY = 'pixio-cart';

function createCartStore() {
  const store = writable<CartItem[]>([]);

  const load = () => {
    if (typeof localStorage === 'undefined') return;

    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) return;

    try {
      const parsed = JSON.parse(raw);

      if (Array.isArray(parsed)) {
        const normalized = parsed
          .map((item) => {
            if (item && typeof item.productId === 'number') {
              return item as CartItem;
            }

            return null;
          })
          .filter((item): item is CartItem => item !== null);

        store.set(normalized);
      }
    } catch (error) {
      console.error('Failed to load cart from localStorage', error);
    }
  };

  const save = (items: CartItem[]) => {
    if (typeof localStorage === 'undefined') return;
    localStorage.setItem(STORAGE_KEY, JSON.stringify(items));
  };

  const subscribe = store.subscribe;

  const setItems = (items: CartItem[]) => {
    store.set(items);
    save(items);
  };

  const addToCart = (product: CartItem) => {
    const items = get(store);

    if (items.some((item) => item.productId === product.productId)) return;

    setItems([...items, product]);
  };

  const removeFromCart = (productId: number) => {
    setItems(get(store).filter((item) => item.productId !== productId));
  };

  const clearCart = () => {
    setItems([]);
  };

  const isInCart = (productId: number) => {
    return get(store).some((item) => item.productId === productId);
  };

  return {
    subscribe,
    load,
    addToCart,
    removeFromCart,
    clearCart,
    isInCart
  };
}

export const cartStore = createCartStore();