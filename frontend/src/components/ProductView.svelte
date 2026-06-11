<script>
  import { onMount } from 'svelte';
  import { productsStore } from '../stores/products';
  import AddToCartButton from './AddToCartButton.svelte';
  import { cartStore } from '../stores/cart';
  import { userStore } from '../stores/user';

  let user = null;
  let productId = null;
  let from = 'catalog';

  let product = null;
  let productImages = [];
  let activeImageIndex = 0;
  let productsLoaded = false;

  $: activeImage = productImages[activeImageIndex] || '';

  const getMainImage = () => {
    return productImages[0] || '';
  };

  const buyNow = () => {
    if (!product) return;

    cartStore.load();

    cartStore.addToCart({
      productId: product.id,
      title: product.title,
      price: product.price,
      category: product.category,
      image: getMainImage()
    });

    window.location.href = '/cart';
  };

  onMount(() => {
    const params = new URLSearchParams(window.location.search);
    productId = params.get('id');
    from = params.get('from') || 'catalog';

    userStore.load();

    const unsubscribeUser = userStore.subscribe((value) => {
      user = value;
    });

    const unsubscribeProducts = productsStore.subscribe((products) => {
      const found = products.find((item) => Number(item.id) === Number(productId)) || null;

      product = found;

      if (found) {
        productImages = Array.isArray(found.images)
          ? found.images.filter(Boolean)
          : [];

        if (activeImageIndex >= productImages.length) {
          activeImageIndex = 0;
        }
      } else {
        productImages = [];
        activeImageIndex = 0;
      }
    });

    productsStore.load().then(() => {
      productsLoaded = true;
    });

    return () => {
      unsubscribeProducts?.();
      unsubscribeUser?.();
    };
  });
</script>

{#if !productsLoaded}
  <div class="rounded-[24px] border border-slate-300/80 bg-[rgba(255,255,255,0.55)] px-6 py-6 backdrop-blur-sm">
    <h1 class="mb-3 text-2xl font-semibold text-slate-900">Загрузка товара...</h1>
    <p class="text-sm text-slate-600">
      Получаем данные товара с сервера.
    </p>
  </div>
{:else if product}
  <div class="mb-6">
    <button
      type="button"
      on:click={() => history.back()}
      class="appearance-none border-0 bg-transparent p-0 text-sm text-slate-500 transition hover:text-slate-900"
    >
      {from === 'seller'
        ? '← Назад к моим товарам'
        : from === 'cart'
          ? '← Назад к корзине'
          : from === 'profile'
            ? '← Назад в профиль'
            : '← Назад в каталог'}
    </button>
  </div>

  <div class="grid gap-5 lg:grid-cols-[1.05fr_0.95fr]">
    <div>
      <div class="overflow-hidden rounded-[24px] bg-[#222744]">
        {#if activeImage}
          <img
            src={activeImage}
            alt={product.title}
            class="h-[420px] w-full object-cover"
          />
        {:else}
          <div class="flex h-[420px] items-center justify-center text-sm text-slate-300">
            Изображение не найдено
          </div>
        {/if}
      </div>

      {#if productImages.length > 1}
        <div class="mt-4 grid grid-cols-7 gap-2">
          {#each productImages as image, index}
            <button
              type="button"
              on:click={() => activeImageIndex = index}
              class={`aspect-square w-full overflow-hidden rounded-[12px] border transition focus:outline-none ${
                index === activeImageIndex
                  ? 'border-slate-500'
                  : 'border-slate-300/80 hover:border-slate-400'
              }`}
              title={`Фото ${index + 1}`}
            >
              <img
                src={image}
                alt={`Фото ${index + 1}`}
                class="h-full w-full object-cover"
              />
            </button>
          {/each}
        </div>
      {/if}
    </div>

    <div class="rounded-[24px] bg-[#e6edf4] px-6 py-6">
      <p class="mb-2 text-sm text-slate-500">{product.category}</p>

      <h1 class="mb-3 text-[2rem] font-semibold leading-tight text-slate-900">
        {product.title}
      </h1>

      <p class="mb-5 text-2xl font-semibold text-slate-900">
        {product.price} ₽
      </p>

      <div class="mb-6 border-t border-slate-300/80 pt-5">
        <p class="text-sm leading-7 text-slate-700">
          {product.description}
        </p>
      </div>

      {#if user?.role !== 'seller'}
        <div class="flex flex-wrap gap-3">
          <AddToCartButton
            id={product.id}
            title={product.title}
            price={product.price}
            category={product.category}
            image={getMainImage()}
          />

          <button
            on:click={buyNow}
            class="rounded-[16px] bg-[rgba(45,65,87,0.08)] px-5 py-3 text-sm font-medium text-slate-900 transition hover:bg-[rgba(45,65,87,0.16)]"
          >
            Купить сейчас
          </button>
        </div>
      {/if}
    </div>
  </div>
{:else}
  <div class="rounded-[24px] border border-slate-300/80 bg-[rgba(255,255,255,0.55)] px-6 py-6 backdrop-blur-sm">
    <h1 class="mb-3 text-2xl font-semibold text-slate-900">Товар не найден</h1>
    <p class="text-sm text-slate-600">
      Для указанного идентификатора товар отсутствует
    </p>
  </div>
{/if}