<script lang="ts">
  import { onMount } from 'svelte';
  import { cartStore, type CartItem } from '../stores/cart';
  import { userStore, type User } from '../stores/user';
  import { ordersStore } from '../stores/orders';
  import { toastStore } from '../stores/toast';

  let cartItems: CartItem[] = [];
  let user: User | null = null;

  let card = '';
  let date = '';
  let cvv = '';
  let error = '';
  let isPaying = false;

  $: total = cartItems.reduce((sum, item) => sum + item.price, 0);

  onMount(() => {
    userStore.load();
    cartStore.load();
    ordersStore.load();

    user = userStore.getCurrentUser();

    if (!user) {
      window.location.href = '/login?redirect=/checkout';
      return;
    }

    if (user.role === 'seller') {
      window.location.href = '/seller/dashboard';
      return;
    }

    const unsubscribeCart = cartStore.subscribe((items) => {
      cartItems = items;
    });

    return () => {
      unsubscribeCart();
    };
  });

  const formatCard = (value: string) => {
    return value
      .replace(/\D/g, '')
      .slice(0, 16)
      .replace(/(.{4})/g, '$1 ')
      .trim();
  };

  const formatDate = (value: string) => {
    const digits = value.replace(/\D/g, '').slice(0, 4);

    if (digits.length <= 2) return digits;

    return `${digits.slice(0, 2)}/${digits.slice(2)}`;
  };

  const handleCardInput = (event: Event) => {
    const target = event.target as HTMLInputElement;
    card = formatCard(target.value);
  };

  const handleDateInput = (event: Event) => {
    const target = event.target as HTMLInputElement;
    date = formatDate(target.value);
  };

  const handleCvvInput = (event: Event) => {
    const target = event.target as HTMLInputElement;
    cvv = target.value.replace(/\D/g, '').slice(0, 3);
  };

  const pay = () => {
    error = '';

    const cardDigits = card.replace(/\D/g, '');

    if (cartItems.length === 0) {
      error = 'Корзина пуста';
      return;
    }

    if (cardDigits.length !== 16) {
      error = 'Введите 16 цифр номера карты';
      return;
    }

    if (!/^\d{2}\/\d{2}$/.test(date)) {
      error = 'Введите срок действия в формате MM/YY';
      return;
    }

    if (cvv.length !== 3) {
      error = 'Введите 3 цифры CVV';
      return;
    }

    if (!user) return;

    const currentUser = user;

    isPaying = true;

    setTimeout(async () => {
      const order = await ordersStore.createOrder(currentUser.id, cartItems);

      if (!order) {
        isPaying = false;
        error = 'Не удалось оформить заказ';
        return;
      }

      cartStore.clearCart();

      toastStore.show('Оплата прошла успешно', 'success');
      toastStore.show('Письмо с ключами отправлено на email', 'info');

      window.location.href = '/order-success';
    }, 900);
  };
</script>

<div class="rounded-[24px] border border-slate-300/80 bg-[rgba(255,255,255,0.55)] px-6 py-6">
  <h1 class="mb-5 text-3xl font-semibold text-slate-900">Оплата</h1>

  <div class="mb-5 rounded-[18px] bg-[rgba(255,255,255,0.48)] px-4 py-4 text-sm leading-6 text-slate-600">
    Это демонстрационная оплата. Реальные платёжные данные не обрабатываются.
  </div>

  <div class="mb-5 space-y-3">
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

  <div class="space-y-4">
    <input
      bind:value={card}
      on:input={handleCardInput}
      placeholder="Номер карты"
      inputmode="numeric"
      maxlength="19"
      class="w-full rounded-[16px] border border-slate-300/80 bg-[rgba(255,255,255,0.72)] px-4 py-3 text-[14px] text-slate-900 outline-none focus:border-slate-500"
    />

    <div class="grid grid-cols-2 gap-3">
      <input
        bind:value={date}
        on:input={handleDateInput}
        placeholder="MM/YY"
        inputmode="numeric"
        maxlength="5"
        class="w-full rounded-[16px] border border-slate-300/80 bg-[rgba(255,255,255,0.72)] px-4 py-3 text-[14px] text-slate-900 outline-none focus:border-slate-500"
      />

      <input
        bind:value={cvv}
        on:input={handleCvvInput}
        placeholder="CVV"
        inputmode="numeric"
        maxlength="3"
        class="w-full rounded-[16px] border border-slate-300/80 bg-[rgba(255,255,255,0.72)] px-4 py-3 text-[14px] text-slate-900 outline-none focus:border-slate-500"
      />
    </div>
  </div>

  {#if error}
    <p class="mt-3 text-sm font-medium text-red-500">{error}</p>
  {/if}

  <button
    type="button"
    on:click={pay}
    disabled={isPaying}
    class="mt-5 w-full rounded-[16px] bg-[#2d4157] px-5 py-3 text-sm font-medium text-white transition hover:bg-[#24364a] disabled:cursor-not-allowed disabled:opacity-70"
  >
    {isPaying ? 'Оплата проходит...' : 'Оплатить'}
  </button>
</div>