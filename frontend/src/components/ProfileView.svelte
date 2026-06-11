<script lang="ts">
  import { onMount } from 'svelte';
  import { userStore, type User } from '../stores/user';
  import { ordersStore, type Order } from '../stores/orders';
  import { toastStore } from '../stores/toast';
  import { productsStore } from '../stores/products';
  import type { Product } from '../types/product';

  let user: User | null = null;
  let orders: Order[] = [];
  let allOrders: Order[] = [];
  let products: Product[] = [];
  let sellerProducts: Product[] = [];

  let isEditing = false;
  let name = '';
  let email = '';
  let selectedItem: Order['items'][number] | null = null;
  let selectedMonth = 'all';
  let monthDropdownOpen = false;

  const formatMonth = (month: string) => {
    if (month === 'all') return 'Все месяцы';

    return new Date(`${month}-01`).toLocaleDateString('ru-RU', {
      month: 'long',
      year: 'numeric'
    });
  };

  const completeOrder = async (orderId: number) => {
    const success = await ordersStore.updateStatus(orderId, 'Выполнен');

    if (!success) {
      toastStore.show('Не удалось обновить статус заказа', 'error');
      return;
    }

    toastStore.show('Заказ подтверждён', 'success');
  };

  const selectMonth = (month: string) => {
    selectedMonth = month;
    monthDropdownOpen = false;
  };

  const getMonthKey = (date: string) => {
    const d = new Date(date);
    const month = String(d.getMonth() + 1).padStart(2, '0');
    return `${d.getFullYear()}-${month}`;
  };

  const copyKey = async () => {
    if (!selectedItem) return;

    await navigator.clipboard.writeText(selectedItem.licenseKey);
    toastStore.show('Ключ скопирован', 'success');
  };

  const openDigitalAccess = (item: Order['items'][number]) => {
    selectedItem = item;
  };

  const closeDigitalAccess = () => {
    selectedItem = null;
  };

  const parseBackendDate = (date: string) => {
    const [datePart, timePart = '00:00:00'] = date.split(' ');
    const cleanTime = timePart.split('.')[0];

    return new Date(`${datePart}T${cleanTime}Z`);
  };

  const formatDate = (date: string) => {
    return new Intl.DateTimeFormat('ru-RU', {
      day: 'numeric',
      month: 'long',
      year: 'numeric',
      hour: '2-digit',
      minute: '2-digit',
      timeZone: 'Europe/Moscow'
    }).format(parseBackendDate(date));
  };

  onMount(() => {
    userStore.load();
    ordersStore.load();
    productsStore.load();

    const unsubscribeUser = userStore.subscribe((value) => {
      user = value;

      if (value) {
        name = value.name;
        email = value.email;
      }
    });

    const unsubscribeOrders = ordersStore.subscribe((value) => {
      allOrders = value;
    });

    const unsubscribeProducts = productsStore.subscribe((value) => {
      products = value;
    });

    return () => {
      unsubscribeUser();
      unsubscribeOrders();
      unsubscribeProducts();
    };
  });

  $: if (user) {
    ordersStore.load();
  }

  $: {
    const currentUser = user;

    orders = currentUser
      ? allOrders.filter((order) => order.userId === currentUser.id)
      : [];
  }

  $: {
    const currentUser = user;

    sellerProducts =
      currentUser?.role === 'seller'
        ? products.filter((product) => product.sellerId === currentUser.id)
        : [];
  }

  $: sellerProductIds = sellerProducts.map((product) => product.id);

  $: filteredOrders =
    selectedMonth === 'all'
      ? allOrders
      : allOrders.filter((order) => getMonthKey(order.date) === selectedMonth);

  $: sellerSoldItems = filteredOrders.flatMap((order) =>
    order.items.filter((item) => sellerProductIds.includes(item.productId))
  );

  $: sellerSalesCount = sellerSoldItems.length;

  $: sellerRevenue = sellerSoldItems.reduce((sum, item) => sum + item.price, 0);

  $: availableMonths = Array.from(
    new Set(allOrders.map((order) => getMonthKey(order.date)))
  );

  const startEdit = () => {
    isEditing = true;
  };

  const cancelEdit = () => {
    if (user) {
      name = user.name;
      email = user.email;
    }

    isEditing = false;
  };

  const saveProfile = async () => {
    if (!name.trim() || !email.trim()) {
      toastStore.show('Заполните имя и email', 'error');
      return;
    }

    const success = await userStore.updateProfile(name, email);

    if (!success) {
      toastStore.show('Такой email уже используется', 'error');
      return;
    }

    toastStore.show('Профиль обновлён', 'success');
    isEditing = false;
  };
</script>

{#if !user}
  <div class="rounded-[24px] border border-slate-300/80 bg-[rgba(255,255,255,0.55)] px-6 py-6">
    <h1 class="mb-2 text-3xl font-semibold text-slate-900">Профиль недоступен</h1>
    <p class="mb-5 text-sm text-slate-600">
      Чтобы посмотреть профиль и историю заказов, войдите в аккаунт.
    </p>

    <a
      href="/login"
      class="inline-flex rounded-[16px] bg-[#2d4157] px-5 py-3 text-sm font-medium text-white transition hover:bg-[#24364a]"
    >
      Войти
    </a>
  </div>
{:else}
  <h1 class="mb-5 text-[2rem] font-semibold leading-tight text-slate-900 sm:mb-6 sm:text-3xl">
    {user.role === 'seller' ? 'Профиль продавца' : 'Профиль покупателя'}
  </h1>

  <div class="grid min-w-0 items-start gap-5 lg:grid-cols-[0.9fr_1.6fr]">
    <div class="min-w-0 self-start rounded-[24px] border border-slate-300/80 bg-[rgba(255,255,255,0.55)] px-4 py-5 backdrop-blur-sm sm:px-6 sm:py-6">
      <div class="mb-4 flex flex-col items-start gap-3 sm:min-h-[44px] sm:flex-row sm:items-center sm:justify-between">
        <h2 class="text-[1.1rem] font-semibold leading-tight text-slate-900 sm:text-[1.2rem]">
          Данные пользователя
        </h2>

        {#if !isEditing}
          <button
            type="button"
            on:click={startEdit}
            class="inline-flex h-9 w-full items-center justify-center rounded-[14px] bg-[rgba(45,65,87,0.08)] px-3.5 text-[13px] text-slate-900 transition hover:bg-[rgba(45,65,87,0.14)] sm:w-auto sm:min-w-[130px]"
          >
            Редактировать
          </button>
        {/if}
      </div>

      {#if !isEditing}
        <div class="space-y-4 text-sm">
          <div>
            <p class="mb-1 text-slate-500">Имя</p>
            <p class="font-medium text-slate-900">{user.name}</p>
          </div>

          <div>
            <p class="mb-1 text-slate-500">Email</p>
            <p class="break-all font-medium text-slate-900">{user.email}</p>
          </div>
        </div>
      {:else}
        <form class="space-y-4">
          <div>
            <label class="mb-1 block text-sm text-slate-500" for="profile-name">
              Имя
            </label>
            <input
              id="profile-name"
              bind:value={name}
              type="text"
              class="w-full rounded-[16px] border border-slate-300/80 bg-[rgba(255,255,255,0.72)] px-4 py-2.5 text-[14px] text-slate-900 outline-none focus:border-slate-500"
            />
          </div>

          <div>
            <label class="mb-1 block text-sm text-slate-500" for="profile-email">
              Email
            </label>
            <input
              id="profile-email"
              bind:value={email}
              type="email"
              class="w-full rounded-[16px] border border-slate-300/80 bg-[rgba(255,255,255,0.72)] px-4 py-3 text-[14px] text-slate-900 outline-none focus:border-slate-500"
            />
          </div>

          <div class="flex flex-wrap gap-2.5 pt-1">
            <button
              type="button"
              on:click={saveProfile}
              class="inline-flex h-9 items-center justify-center rounded-[16px] bg-[#2d4157] px-4 text-[11px] font-medium text-white transition hover:bg-[#24364a]"
            >
              Сохранить
            </button>

            <button
              type="button"
              on:click={cancelEdit}
              class="inline-flex h-9 items-center justify-center rounded-[16px] bg-[rgba(45,65,87,0.08)] px-4 text-[11px] text-slate-900 transition hover:bg-[rgba(45,65,87,0.14)]"
            >
              Отмена
            </button>
          </div>
        </form>
      {/if}
    </div>

    <div class="min-w-0 rounded-[24px] border border-slate-300/80 bg-[rgba(255,255,255,0.55)] px-4 py-5 backdrop-blur-sm sm:px-6 sm:py-6">
      <h2 class="mb-5 text-xl font-semibold text-slate-900">
        {user.role === 'seller' ? 'Информация продавца' : 'Мои заказы'}
      </h2>

      {#if user.role === 'seller'}
        <div class="grid grid-cols-1 gap-4 sm:grid-cols-2 xl:grid-cols-3">
          <div class="rounded-[18px] bg-[rgba(255,255,255,0.55)] px-4 py-4">
            <p class="mb-2 text-sm text-slate-500">Товаров</p>
            <p class="text-2xl font-semibold text-slate-900">{sellerProducts.length}</p>
          </div>

          <div class="rounded-[18px] bg-[rgba(255,255,255,0.55)] px-4 py-4">
            <p class="mb-2 text-sm text-slate-500">Продаж</p>
            <p class="text-2xl font-semibold text-slate-900">{sellerSalesCount}</p>
          </div>

          <div class="rounded-[18px] bg-[rgba(255,255,255,0.55)] px-4 py-4">
            <p class="mb-2 text-sm text-slate-500">Выручка</p>
            <p class="text-2xl font-semibold text-slate-900">{sellerRevenue} ₽</p>
          </div>
        </div>
        <div class="mt-5">
          <div class="mb-3 flex flex-col gap-3 sm:flex-row sm:items-center sm:justify-between">
            <h3 class="text-base font-semibold text-slate-900">Проданные товары</h3>

            <div class="relative w-full sm:w-auto">
              <button
                type="button"
                on:click={() => (monthDropdownOpen = !monthDropdownOpen)}
                class="flex h-10 w-full items-center justify-between gap-3 rounded-[14px] border border-slate-300/80 bg-[rgba(255,255,255,0.72)] px-4 text-sm text-slate-900 transition hover:border-slate-500 sm:min-w-[180px]"
              >
                <span>{formatMonth(selectedMonth)}</span>
                <span class="text-xs text-slate-500">▼</span>
              </button>

              {#if monthDropdownOpen}
                <div class="absolute right-0 top-full z-30 mt-2 w-full overflow-hidden rounded-[14px] border border-slate-300/80 bg-white shadow-[0_10px_28px_rgba(0,0,0,0.12)]">
                  <button
                    type="button"
                    on:click={() => selectMonth('all')}
                    class="block w-full px-4 py-2.5 text-left text-sm text-slate-900 transition hover:bg-[rgba(45,65,87,0.08)]"
                  >
                    Все месяцы
                  </button>

                  {#each availableMonths as month}
                    <button
                      type="button"
                      on:click={() => selectMonth(month)}
                      class="block w-full px-4 py-2.5 text-left text-sm text-slate-900 transition hover:bg-[rgba(45,65,87,0.08)]"
                    >
                      {formatMonth(month)}
                    </button>
                  {/each}
                </div>
              {/if}
            </div>
          </div>

        {#if sellerSoldItems.length === 0}
          <p class="text-sm text-slate-600">Продаж пока нет</p>
        {:else}
          <div class="space-y-2">
            {#each sellerSoldItems as item}
              <div class="flex items-center justify-between gap-4 rounded-[14px] bg-[rgba(255,255,255,0.48)] px-4 py-3 text-sm">
                <span class="text-slate-900">{item.title}</span>
                <span class="shrink-0 font-medium text-slate-900">{item.price} ₽</span>
              </div>
            {/each}
          </div>
        {/if}
      </div>
      {:else if orders.length === 0}
        <p class="text-sm leading-6 text-slate-600">
          У вас пока нет заказов.
        </p>
      {:else}
        <div class="space-y-4">
          {#each orders as order}
            <details class="overflow-hidden rounded-[18px] border border-slate-300/80 bg-[rgba(255,255,255,0.42)]">
              <summary class="cursor-pointer list-none px-4 py-4">
                <div class="flex flex-col gap-4 sm:flex-row sm:items-start sm:justify-between">
                  <div>
                    <p class="text-base font-medium text-slate-900">Заказ №{order.id}</p>
                    <p class="mt-1 text-sm text-slate-600">Сумма: {order.total} ₽</p>
                    <p class="mt-1 text-sm text-slate-500">
                      Дата покупки: {formatDate(order.date)}
                    </p>
                  </div>

                  <div class="flex items-start justify-between gap-4 sm:justify-start">
                    <div class="text-left sm:text-right">
                      <p class="text-sm text-slate-500">Статус</p>
                      <p class="font-medium text-slate-900">{order.status}</p>

                      {#if order.status === 'Оплачен'}
                        <button
                          on:click={() => completeOrder(order.id)}
                          class="mt-2 w-full rounded-[12px] bg-[rgba(45,65,87,0.08)] px-3 py-1.5 text-xs text-slate-900 transition hover:bg-[rgba(45,65,87,0.14)] sm:w-auto"
                        >
                          Подтвердить получение
                        </button>
                      {/if}
                    </div>

                    <span class="details-arrow mt-1 text-sm text-slate-500 transition">▼</span>
                  </div>
                </div>
              </summary>

              <div class="border-t border-slate-300/80 px-4 py-4">
                <p class="mb-3 text-sm font-medium text-slate-700">Товары в заказе</p>

                <div class="space-y-1">
                  {#each order.items as item}
                    <div
                      class="flex flex-col gap-3 rounded-[14px] px-3 py-3 text-sm transition hover:bg-[rgba(45,65,87,0.08)] sm:flex-row sm:items-center sm:justify-between sm:px-4 sm:py-2.5"
                    >
                      <a
                        href={`/product/view?id=${item.productId}&from=profile`}
                        class="min-w-0 flex-1 text-slate-900 hover:underline"
                      >
                        {item.title}
                      </a>

                      <div class="flex w-full flex-col gap-2 sm:w-auto sm:shrink-0 sm:flex-row sm:items-center sm:gap-3">
                      <span class="font-medium text-slate-900">{item.price} ₽</span>

                      {#if order.status === 'Выполнен'}
                        <button
                          type="button"
                          on:click={() => openDigitalAccess(item)}
                          class="w-full rounded-[14px] bg-[#2d4157] px-4 py-2 text-xs font-medium text-white transition hover:bg-[#24364a] sm:w-auto"
                        >
                          Получить товар
                        </button>
                      {:else}
                        <button
                          type="button"
                          disabled
                          class="w-full cursor-not-allowed rounded-[14px] bg-[rgba(45,65,87,0.08)] px-4 py-2 text-xs font-medium text-slate-500 sm:w-auto"
                        >
                          Доступ после подтверждения
                        </button>
                       {/if}
                      </div>
                    </div>
                  {/each}
                </div>
              </div>
            </details>
          {/each}
        </div>
      {/if}
    </div>
  </div>
{/if}

{#if selectedItem}
  <div class="fixed inset-0 z-[100] flex items-center justify-center bg-slate-950/45 px-5 backdrop-blur-sm">
    <div class="w-full max-w-md rounded-[24px] border border-slate-300/80 bg-[#eef4fa] px-4 py-5 shadow-[0_18px_60px_rgba(0,0,0,0.18)] sm:px-6 sm:py-6">
      <div class="mb-5 flex items-start justify-between gap-4">
        <div>
          <h2 class="text-2xl font-semibold text-slate-900">Доступ к товару</h2>
          <p class="mt-2 text-sm leading-6 text-slate-600">
            Цифровой товар доступен в личном кабинете. Дополнительно данные доступа могут быть отправлены на email.
          </p>
        </div>

        <button
          type="button"
          on:click={closeDigitalAccess}
          class="text-2xl leading-none text-slate-500 transition hover:text-slate-900"
        >
          ×
        </button>
      </div>

      <div class="mb-5 rounded-[18px] bg-[rgba(255,255,255,0.55)] px-4 py-4">
        <p class="mb-1 text-sm text-slate-500">Товар</p>
        <p class="mb-4 font-medium text-slate-900">{selectedItem.title}</p>

        <p class="mb-1 text-sm text-slate-500">Лицензионный ключ</p>
        <div class="flex items-center gap-2 rounded-[14px] bg-[#2d4157] px-4 py-3">
          <p class="min-w-0 flex-1 break-all font-mono text-sm font-semibold text-white">
            {selectedItem.licenseKey}
          </p>

          <button
            type="button"
            on:click={copyKey}
            class="inline-flex h-8 w-8 shrink-0 items-center justify-center rounded-[10px] text-white/75 transition hover:bg-white/10 hover:text-white"
            title="Скопировать ключ"
            aria-label="Скопировать ключ"
          >
            <svg
              class="h-5 w-5"
              xmlns="http://www.w3.org/2000/svg"
              fill="none"
              viewBox="0 0 24 24"
              stroke="currentColor"
              stroke-width="1.8"
            >
              <rect x="9" y="9" width="10" height="10" rx="2" />
              <rect x="5" y="5" width="10" height="10" rx="2" />
            </svg>
          </button>
        </div>
      </div>

      <button
        type="button"
        on:click={closeDigitalAccess}
        class="w-full rounded-[16px] bg-[#2d4157] px-5 py-3 text-sm font-medium text-white transition hover:bg-[#24364a]"
      >
        Готово
      </button>
    </div>
  </div>
{/if}

<style>
  details[open] .details-arrow {
    transform: rotate(180deg);
  }

  summary::-webkit-details-marker {
    display: none;
  }
</style>