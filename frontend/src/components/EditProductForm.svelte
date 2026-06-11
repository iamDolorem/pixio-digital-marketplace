<script>
  import { onMount } from 'svelte';
  import { productsStore } from '../stores/products';
  import { userStore } from '../stores/user';

  let productId = null;
  let product = null;

  let title = '';
  let category = '';
  let price = '';
  let imageUrls = [''];
  let activePreviewIndex = 0;
  let description = '';

  let titleField;
  let descriptionField;
  let categoryMenu;
  let categoryLabel;

  const categories = ['Игры', 'Артбуки', 'Музыка', 'Графика', 'Шаблоны', 'Интерфейсы'];

  const autoResize = (element) => {
    if (!element) return;
    element.style.height = 'auto';
    element.style.height = element.scrollHeight + 'px';
  };

  $: normalizedImageUrls = imageUrls
    .map((url) => url.trim())
    .filter(Boolean);

  $: if (normalizedImageUrls.length > 0 && activePreviewIndex >= normalizedImageUrls.length) {
    activePreviewIndex = normalizedImageUrls.length - 1;
  }

  $: if (normalizedImageUrls.length === 0 && activePreviewIndex !== 0) {
    activePreviewIndex = 0;
  }

  $: previewImage = normalizedImageUrls[activePreviewIndex] || '';

  const normalizeImages = () => {
    return imageUrls.map((url) => url.trim()).filter(Boolean);
  };

  const addImageField = () => {
    imageUrls = [...imageUrls, ''];
    activePreviewIndex = imageUrls.length - 1;
  };

  const removeImageField = (index) => {
    if (imageUrls.length === 1) {
      imageUrls = [''];
      activePreviewIndex = 0;
      return;
    }

    const updated = imageUrls.filter((_, currentIndex) => currentIndex !== index);

    imageUrls = updated;
    activePreviewIndex = Math.min(activePreviewIndex, updated.length - 1);
  };

  const updateImageUrl = (index, value) => {
    activePreviewIndex = index;

    imageUrls = imageUrls.map((url, currentIndex) =>
      currentIndex === index ? value : url
    );
  };

  onMount(() => {
    const params = new URLSearchParams(window.location.search);
    productId = params.get('id');

    userStore.load();
    productsStore.load();

    const currentUser = userStore.getCurrentUser();

    if (!currentUser) {
      window.location.href = `/login?redirect=${window.location.pathname}${window.location.search}`;
      return;
    }

    if (currentUser.role !== 'seller') {
      window.location.href = '/';
      return;
    }

    const unsubscribe = productsStore.subscribe((products) => {
      const found = products.find((item) => item.id === Number(productId)) || null;

      if (found && found.sellerId !== currentUser.id) {
        window.location.href = '/seller/dashboard';
        return;
      }

      product = found;

      if (found) {
        title = found.title;
        category = found.category;
        price = String(found.price);
        imageUrls = found.images && found.images.length > 0
          ? [...found.images]
          : [''];

        description = found.description;

        setTimeout(() => {
          autoResize(titleField);
          autoResize(descriptionField);
        });
      }
    });

    return () => unsubscribe?.();
  });

  const toggleCategoryMenu = () => {
    categoryMenu?.classList.toggle('hidden');
  };

  const selectCategory = (value) => {
    category = value;
    categoryMenu?.classList.add('hidden');
  };

  const saveProduct = async () => {
    const parsedPrice = Number(price);
    const images = normalizeImages();

    if (
      !title.trim() ||
      !category.trim() ||
      !String(price).trim() ||
      Number.isNaN(parsedPrice) ||
      parsedPrice < 0 ||
      images.length === 0 ||
      !description.trim()
    ) {
      alert('Заполните все поля корректно. Минимум одно изображение обязательно.');
      return;
    }

    const success = await productsStore.updateProduct(Number(productId), {
      title: title.trim(),
      category: category.trim(),
      price: parsedPrice,
      images,
      description: description.trim()
    });

    if (!success) {
      alert('Не удалось сохранить товар.');
      return;
    }

    window.location.href = '/seller/dashboard';
  };
</script>

{#if product}
  <div class="grid items-start gap-5 lg:grid-cols-[1fr_1fr]">
    <div class="rounded-[24px] border border-slate-300/80 bg-[rgba(255,255,255,0.55)] p-4 backdrop-blur-sm">
      <div class="overflow-hidden rounded-[18px] bg-[#222744]">
        {#if previewImage}
          <img
            src={previewImage}
            alt={title}
            class="h-[360px] w-full object-cover"
          />
        {:else}
          <div class="flex h-[360px] items-center justify-center text-sm text-slate-500">
            Предпросмотр изображения
          </div>
        {/if}
      </div>

      {#if normalizedImageUrls.length > 0}
        <div class="mt-4 grid grid-cols-7 gap-2">
          {#each normalizedImageUrls as url, index}
            <button
              type="button"
              on:click={() => activePreviewIndex = index}
              class={`aspect-square w-full overflow-hidden rounded-[12px] border transition focus:outline-none ${
                index === activePreviewIndex
                  ? 'border-slate-500'
                  : 'border-slate-300/80 hover:border-slate-400'
              }`}
              title={`Фото ${index + 1}`}
            >
              <img
                src={url}
                alt={`Фото ${index + 1}`}
                class="h-full w-full object-cover"
              />
            </button>
          {/each}
        </div>
      {/if}
    </div>

    <div class="rounded-[24px] border border-slate-300/80 bg-[rgba(255,255,255,0.55)] px-6 py-6 backdrop-blur-sm">
      <form class="space-y-5" on:submit|preventDefault={saveProduct}>
        <div>
          <label for="title-field" class="mb-2 block text-sm font-medium text-slate-700">
            Название товара
          </label>
          <textarea
            id="title-field"
            bind:this={titleField}
            bind:value={title}
            rows="1"
            class="w-full resize-none overflow-hidden rounded-[16px] border border-slate-300/80 bg-[rgba(255,255,255,0.72)] px-4 py-3 text-[14px] text-slate-900 outline-none focus:border-slate-500"
            on:input={() => autoResize(titleField)}
          ></textarea>
        </div>

        <div>
          <p class="mb-2 block text-sm font-medium text-slate-700">
            Категория
          </p>

          <div class="relative">
            <button
              type="button"
              on:click={toggleCategoryMenu}
              class="flex w-full items-center justify-between rounded-[16px] border border-slate-300/80 bg-[rgba(255,255,255,0.72)] px-4 py-3 text-[14px] text-slate-900 transition hover:border-slate-500"
            >
              <span bind:this={categoryLabel} class:text-slate-500={!category} class:text-slate-900={!!category}>
                {category || 'Выберите категорию'}
              </span>
              <span class="text-xs text-slate-500">▼</span>
            </button>

            <div
              bind:this={categoryMenu}
              class="absolute left-0 top-full z-20 mt-2 hidden w-full overflow-hidden rounded-[16px] border border-slate-300/80 bg-white shadow-[0_8px_24px_rgba(0,0,0,0.08)]"
            >
              {#each categories as option}
                <button
                  type="button"
                  on:click={() => selectCategory(option)}
                  class="block w-full px-4 py-2 text-left text-[14px] text-slate-900 transition hover:bg-[rgba(45,65,87,0.08)]"
                >
                  {option}
                </button>
              {/each}
            </div>
          </div>
        </div>

        <div>
          <label for="price-field" class="mb-2 block text-sm font-medium text-slate-700">
            Цена
          </label>
          <input
            bind:value={price}
            type="number"
            min="0"
            step="1"
            class="w-full rounded-[16px] border border-slate-300/80 bg-[rgba(255,255,255,0.72)] px-4 py-3 text-[14px] text-slate-900 outline-none focus:border-slate-500"
          />
        </div>

        <div>
          <div class="mb-2 flex items-center justify-between gap-3">
            <div>
              <p class="text-sm font-medium text-slate-700">Изображения товара</p>
              <p class="mt-1 text-xs text-slate-500">
                Фото 1 — основная обложка, фото 2 — изображение при наведении, остальные фото отображаются на странице товара
              </p>
            </div>

            <button
              type="button"
              on:click={addImageField}
              class="shrink-0 rounded-[14px] bg-[rgba(45,65,87,0.08)] px-4 py-2 text-xs text-slate-900 transition hover:bg-[rgba(45,65,87,0.14)]"
            >
              Добавить фото
            </button>
          </div>

          <div class="space-y-3">
            {#each imageUrls as url, index}
              <div class="flex gap-2">
                <input
                  value={url}
                  type="text"
                  placeholder={`Ссылка на фото ${index + 1}`}
                  class="w-full rounded-[16px] border border-slate-300/80 bg-[rgba(255,255,255,0.72)] px-4 py-3 text-[14px] text-slate-900 outline-none focus:border-slate-500"
                  on:focus={() => activePreviewIndex = index}
                  on:input={(event) => updateImageUrl(index, event.currentTarget.value)}
                />

                <button
                  type="button"
                  on:click={() => removeImageField(index)}
                  class="rounded-[16px] bg-[rgba(45,65,87,0.08)] px-4 py-3 text-sm text-slate-900 transition hover:bg-[rgba(45,65,87,0.14)]"
                >
                  ×
                </button>
              </div>
            {/each}
          </div>
        </div>

        <div>
          <label for="description-field" class="mb-2 block text-sm font-medium text-slate-700">
            Описание
          </label>
          <textarea
            bind:this={descriptionField}
            bind:value={description}
            rows="1"
            class="w-full resize-none overflow-hidden rounded-[16px] border border-slate-300/80 bg-[rgba(255,255,255,0.72)] px-4 py-3 text-[14px] text-slate-900 outline-none focus:border-slate-500"
            on:input={() => autoResize(descriptionField)}
          ></textarea>
        </div>

        <div class="flex flex-wrap gap-3 pt-1">
          <button
            type="submit"
            class="rounded-[16px] bg-[#2d4157] px-5 py-3 text-sm font-medium text-white transition hover:bg-[#24364a]"
          >
            Сохранить
          </button>

          <a
            href="/seller/dashboard"
            class="rounded-[16px] bg-[rgba(45,65,87,0.08)] px-5 py-3 text-sm text-slate-900 transition hover:bg-[rgba(45,65,87,0.14)]"
          >
            Отмена
          </a>
        </div>
      </form>
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