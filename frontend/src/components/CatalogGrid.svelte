<script lang="ts">
  import { onMount } from 'svelte';
  import { productsStore } from '../stores/products';
  import type { Product } from '../types/product';
  import ProductCard from './ProductCard.svelte';

  let products: Product[] = [];
  let productsLoaded = false;

  let search = '';
  let category = '';
  let minPrice = '';
  let maxPrice = '';
  let sort = '';

  $: filteredProducts = products;

  onMount(() => {
    const unsubscribe = productsStore.subscribe((value) => {
      products = value;
    });

    productsStore.load().then(() => {
      productsLoaded = true;
    });

    let filterTimer: ReturnType<typeof setTimeout> | null = null;

    const handleFiltersChange = (event: Event) => {
      const customEvent = event as CustomEvent;

      search = customEvent.detail.search || '';
      category = customEvent.detail.category || '';
      minPrice = customEvent.detail.minPrice || '';
      maxPrice = customEvent.detail.maxPrice || '';
      sort = customEvent.detail.sort || '';

      if (filterTimer) {
        clearTimeout(filterTimer);
      }

      filterTimer = setTimeout(() => {
        productsStore.load({
          search,
          category,
          minPrice,
          maxPrice,
          sort
        });
      }, 300);
    };

    window.addEventListener('catalog-filters-change', handleFiltersChange);

    return () => {
      unsubscribe();
      window.removeEventListener('catalog-filters-change', handleFiltersChange);

      if (filterTimer) {
        clearTimeout(filterTimer);
      }
    };
  });
</script>

{#if !productsLoaded}
  <div class="px-2 py-6">
    <h2 class="mb-2 text-2xl font-semibold text-slate-900">Загрузка товаров...</h2>
    <p class="text-sm text-slate-600">
      Получаем каталог с сервера
    </p>
  </div>
{:else if filteredProducts.length === 0}
  <div class="px-2 py-6">
    <h2 class="mb-2 text-2xl font-semibold text-slate-900">Товары не найдены</h2>
    <p class="text-sm text-slate-600">
      Попробуйте изменить поиск, категорию, цену или сортировку
    </p>
  </div>
{:else}
  <div class="grid grid-cols-1 gap-x-5 gap-y-8 sm:grid-cols-2 xl:grid-cols-3">
    {#each filteredProducts as product}
      <ProductCard
        product={product}
        href={`/product/view?id=${product.id}&from=catalog`}
      />
    {/each}
  </div>
{/if}