<script lang="ts">
  import { onMount } from 'svelte';
  import { cartStore, type CartItem } from '../stores/cart';
  import { productsStore } from '../stores/products';
  import type { Product } from '../types/product';
  import ConfirmModal from './ConfirmModal.svelte';
  import { ordersStore } from '../stores/orders';
  import { userStore, type User } from '../stores/user';
  import { toastStore } from '../stores/toast';

  let cartItems: CartItem[] = [];
  let products: Product[] = [];
  let user: User | null = null;
  let showClearConfirm = false;
  let productsLoaded = false;

  $: cartItemsWithAvailability = cartItems.map((item) => {
    const isAvailable =
      !productsLoaded ||
      products.some((product) => Number(product.id) === Number(item.productId));

    return {
      ...item,
      isAvailable
    };
  });

  $: hasUnavailableItems =
    productsLoaded && cartItemsWithAvailability.some((item) => !item.isAvailable);

  $: total = cartItems.reduce((sum, item) => sum + item.price, 0);

  onMount(() => {
    userStore.load();
    ordersStore.load();

    const unsubscribeUser = userStore.subscribe((value) => {
      user = value;

      if (value?.role === 'seller') {
        window.location.href = '/seller/dashboard';
      }
    });

    const unsubscribeProducts = productsStore.subscribe((value) => {
      products = value;
    });

    productsStore.load().then(() => {
      productsLoaded = true;
    });

    cartStore.load();

    const unsubscribeCart = cartStore.subscribe((value) => {
    cartItems = value;
    });

    return () => {
      unsubscribeProducts();
      unsubscribeCart();
      unsubscribeUser();
    };
  });

  const checkout = () => {
    if (!user) {
      window.location.href = '/login?redirect=/cart';
      return;
    }

    if (cartItems.length === 0) return;

    if (hasUnavailableItems) {
      toastStore.show('Удалите недоступные товары из корзины', 'error');
      return;
    }

    window.location.href = '/checkout';
  };

  const remove = (productId: number) => {
    cartStore.removeFromCart(productId);
    toastStore.show('Товар удалён из корзины', 'info');
  };

  const clear = () => {
    showClearConfirm = true;
  };

  const confirmClear = () => {
    cartStore.clearCart();
    showClearConfirm = false;
    toastStore.show('Корзина очищена', 'info');
  };

  const cancelClear = () => {
    showClearConfirm = false;
  };
</script>

{#if cartItems.length === 0}
  <div class="rounded-[24px] border border-slate-300/80 bg-[rgba(255,255,255,0.55)] px-6 py-6">
    <h1 class="mb-2 text-3xl font-semibold text-slate-900">Корзина пуста</h1>
    <p class="mb-5 text-sm text-slate-600">
      Добавьте товары из каталога, чтобы оформить заказ
    </p>

    <a
      href="/"
      class="inline-flex items-center rounded-[16px] bg-[#2d4157] px-5 py-3 text-sm font-medium text-white transition hover:bg-[#24364a]"
    >
      Перейти в каталог
    </a>
  </div>
{:else}
  <div class="mb-5 flex items-start justify-between gap-4">
    <div>
      <h1 class="mb-2 text-3xl font-semibold text-slate-900">Корзина</h1>
      <p class="text-sm text-slate-500">
        {cartItems.length} {cartItems.length === 1 ? 'товар' : 'товара'}
      </p>
    </div>

    <button
      on:click={clear}
      class="rounded-[16px] bg-transparent px-3 py-2 text-sm text-slate-500 transition hover:bg-[rgba(45,65,87,0.08)] hover:text-slate-900"
    >
      Очистить корзину
    </button>
  </div>

  <div class="grid items-start gap-5 lg:grid-cols-[1.55fr_0.85fr]">
    <div class="space-y-3">
      {#each cartItemsWithAvailability as item}
        <div class="group rounded-[24px] border border-slate-300/80 bg-[rgba(255,255,255,0.55)] px-5 py-5 transition hover:bg-[rgba(255,255,255,0.80)]">
          <div class="grid items-center gap-4 md:grid-cols-[110px_1fr_auto]">
            <a
              href={`/product/view?id=${item.productId}&from=cart`}
              class="overflow-hidden rounded-[16px] bg-[#222744]"
            >
              <img
                src={item.image}
                alt={item.title}
                class="h-24 w-full object-cover"
              />
            </a>

            <div>
              <a
                href={item.isAvailable ? `/product/view?id=${item.productId}&from=cart` : '#'}
                class="mb-1 block text-[1.05rem] font-medium leading-6 text-slate-900 transition hover:underline"
              >
                {item.title}
              </a>

              <p class="mb-2 text-sm text-slate-500">{item.category}</p>

              {#if productsLoaded && !item.isAvailable}
                <p class="mb-2 text-sm font-medium text-red-500">
                  Товар больше недоступен в каталоге
                </p>
              {/if}

              <p class="text-xl font-semibold text-slate-900">
                {item.price} ₽
              </p>
            </div>

            <button
              on:click={() => remove(item.productId)}
              class="rounded-[16px] bg-transparent px-3 py-2 text-sm text-slate-500 transition hover:bg-[rgba(45,65,87,0.08)] hover:text-slate-900"
            >
              Удалить
            </button>
          </div>
        </div>
      {/each}
    </div>

    <div class="h-fit rounded-[24px] border border-slate-300/80 bg-[rgba(255,255,255,0.55)] px-6 py-6">
      <h2 class="mb-5 text-2xl font-semibold text-slate-900">Итого</h2>

      <div class="mb-4 space-y-3">
        {#each cartItems as item}
          <div class="flex items-start justify-between gap-4 text-sm text-slate-700">
            <span>{item.title}</span>
            <span class="shrink-0">{item.price} ₽</span>
          </div>
        {/each}
      </div>

      <div class="mb-5 border-t border-slate-300/80 pt-4">
        <div class="flex items-center justify-between gap-4">
          <span class="text-[1.05rem] text-slate-700">Сумма заказа</span>
          <span class="text-[1.7rem] font-semibold text-slate-900">{total} ₽</span>
        </div>
      </div>

      <button
        on:click={checkout}
        class="w-full rounded-[18px] bg-[#2d4157] px-5 py-3 text-base font-medium text-white transition hover:bg-[#24364a]"
      >
        Оформить заказ
      </button>
    </div>
  </div>
{/if}

{#if showClearConfirm}
  <ConfirmModal
    title="Очистить корзину?"
    text="Все товары будут удалены из корзины"
    confirmText="Очистить"
    cancelText="Отмена"
    onConfirm={confirmClear}
    onCancel={cancelClear}
  />
{/if}