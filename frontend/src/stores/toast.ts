import { writable } from 'svelte/store';

export type Toast = {
  id: number;
  text: string;
  type?: 'success' | 'info' | 'error';
};

function createToastStore() {
  const store = writable<Toast[]>([]);

  const show = (text: string, type: Toast['type'] = 'info') => {
    const id = Date.now();

    store.update((toasts) => [
      ...toasts,
      {
        id,
        text,
        type
      }
    ]);

    setTimeout(() => {
      store.update((toasts) => toasts.filter((toast) => toast.id !== id));
    }, 2500);
  };

  const remove = (id: number) => {
    store.update((toasts) => toasts.filter((toast) => toast.id !== id));
  };

  return {
    subscribe: store.subscribe,
    show,
    remove
  };
}

export const toastStore = createToastStore();