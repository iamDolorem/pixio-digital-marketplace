import { writable, get } from 'svelte/store';
import type { Product } from '../types/product';

type BackendProduct = {
  id: number;
  sellerId: number;
  title: string;
  category: string;
  description: string;
  price: number;
  image: string;
  hoverImage?: string;
  images?: string[];
};

export type ProductFilters = {
  search?: string;
  category?: string;
  minPrice?: string;
  maxPrice?: string;
  sort?: string;
};

const API_URL = 'http://192.168.1.87:5555/api/products';

function createProductsStore() {
  const store = writable<Product[]>([]);

  const normalizeProduct = (product: BackendProduct): Product => {
    const legacyImage = String(product.image || '');
    const legacyHoverImage = String(product.hoverImage || '');

    const imagesFromBackend = Array.isArray(product.images)
      ? product.images.map((url) => String(url)).filter(Boolean)
      : [];

    const images = imagesFromBackend.length > 0
      ? imagesFromBackend
      : [legacyImage, legacyHoverImage].filter(Boolean);

    return {
      id: Number(product.id),
      sellerId: Number(product.sellerId),
      title: String(product.title),
      category: String(product.category),
      description: String(product.description),
      price: Number(product.price),
      images
    };
  };

  const load = async (filters: ProductFilters = {}) => {
    try {
      const params = new URLSearchParams();

      if (filters.search?.trim()) {
        params.set('search', filters.search.trim());
      }

      if (filters.category?.trim()) {
        params.set('category', filters.category.trim());
      }

      if (filters.minPrice?.trim()) {
        params.set('minPrice', filters.minPrice.trim());
      }

      if (filters.maxPrice?.trim()) {
        params.set('maxPrice', filters.maxPrice.trim());
      }

      if (filters.sort?.trim()) {
        params.set('sort', filters.sort.trim());
      }

      const url = params.toString()
        ? `${API_URL}?${params.toString()}`
        : API_URL;

      const response = await fetch(url);

      if (!response.ok) {
        console.error('Failed to load products from backend');
        return false;
      }

      const data = await response.json();

      if (!data.success || !Array.isArray(data.products)) {
        console.error('Invalid products response', data);
        return false;
      }

      const products: Product[] = data.products.map((product: BackendProduct) =>
        normalizeProduct(product)
      );

      store.set(products);

      return true;
    } catch (error) {
      console.error('Failed to load products from backend', error);
      return false;
    }
  };

  const subscribe = store.subscribe;

  const addProduct = async (product: Omit<Product, 'id'>) => {
    try {
      const images = product.images || [];

      const payload = {
        sellerId: product.sellerId,
        title: product.title,
        category: product.category,
        description: product.description,
        price: product.price,
        images,
        image: images[0] || '',
        hoverImage: images[1] || ''
      };

      const response = await fetch(API_URL, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(payload)
      });

      const data = await response.json();

      if (!response.ok || !data.success || !data.product) {
        console.error('Failed to create product', data);
        return null;
      }

      const newProduct = normalizeProduct(data.product);

      store.set([...get(store), newProduct]);

      return newProduct;
    } catch (error) {
      console.error('Failed to create product through backend', error);
      return null;
    }
  };

  const updateProduct = async (id: number, updatedFields: Partial<Product>) => {
    try {
      const oldProduct = get(store).find((product) => product.id === id);

      if (!oldProduct) {
        console.error('Product not found in store');
        return false;
      }

      const images = updatedFields.images ?? oldProduct.images;

      const payload = {
        title: updatedFields.title ?? oldProduct.title,
        category: updatedFields.category ?? oldProduct.category,
        description: updatedFields.description ?? oldProduct.description,
        price: updatedFields.price ?? oldProduct.price,
        images,
        image: images[0] || '',
        hoverImage: images[1] || ''
      };

      const response = await fetch(`${API_URL}/${id}`, {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(payload)
      });

      const data = await response.json();

      if (!response.ok || !data.success || !data.product) {
        console.error('Failed to update product', data);
        return false;
      }

      const updatedProduct = normalizeProduct(data.product);

      const products = get(store);
      const updated = products.map((product) =>
        product.id === id ? updatedProduct : product
      );

      store.set(updated);

      return true;
    } catch (error) {
      console.error('Failed to update product through backend', error);
      return false;
    }
  };

  const deleteProduct = async (id: number) => {
    try {
      const response = await fetch(`${API_URL}/${id}`, {
        method: 'DELETE'
      });

      const data = await response.json();

      if (!response.ok || !data.success) {
        console.error('Failed to delete product', data);
        return false;
      }

      const products = get(store);
      store.set(products.filter((product) => product.id !== id));

      return true;
    } catch (error) {
      console.error('Failed to delete product through backend', error);
      return false;
    }
  };

  const getProductById = (id: number) => {
    return get(store).find((product) => product.id === id);
  };

  const resetProducts = () => {
    store.set([]);
    load();
  };

  return {
    subscribe,
    load,
    addProduct,
    updateProduct,
    deleteProduct,
    getProductById,
    resetProducts
  };
}

export const productsStore = createProductsStore();