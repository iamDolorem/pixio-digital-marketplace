<script lang="ts">
  import type { Product } from '../types/product';

  export let product: Product;
  export let href = '#';
  export let showActions = false;
  export let editHref = '#';
  export let onDelete: (() => void) | null = null;

  $: mainImage = product.images?.[0] || '';
  $: hoverImage = product.images?.[1] || '';
</script>

<div>
  <a href={href} class="group block">
    <div class="overflow-hidden rounded-[16px] bg-[#222744]">
      <div class="relative aspect-video overflow-hidden">
        {#if mainImage}
          <img
            src={mainImage}
            alt={product.title}
            class={`absolute inset-0 h-full w-full object-cover transition duration-300 ${
              hoverImage ? 'group-hover:opacity-0' : ''
            }`}
          />
        {:else}
          <div class="flex h-full w-full items-center justify-center text-sm text-slate-300">
            Нет изображения
          </div>
        {/if}

        {#if hoverImage}
          <img
            src={hoverImage}
            alt={product.title}
            class="absolute inset-0 h-full w-full object-cover opacity-0 transition duration-300 group-hover:opacity-100"
          />
        {/if}
      </div>
    </div>

    <div class="pt-4">
      <h3 class="mb-2 text-[1.05rem] font-medium leading-7 text-slate-900">
        {product.title}
      </h3>

      <p class="mb-2 text-sm text-slate-500">{product.category}</p>

      <p class="text-xl font-semibold text-slate-900">{product.price} ₽</p>
    </div>
  </a>

  {#if showActions}
    <div class="flex gap-2 pt-4">
      <a
        href={editHref}
        class="rounded-[16px] bg-[#2d4157] px-4 py-2.5 text-sm font-medium text-white transition hover:bg-[#24364a]"
      >
        Редактировать
      </a>

      <button
        type="button"
        on:click={() => onDelete && onDelete()}
        class="rounded-[16px] bg-[rgba(45,65,87,0.08)] px-4 py-2.5 text-sm text-slate-900 transition hover:bg-[rgba(45,65,87,0.16)]"
      >
        Удалить
      </button>
    </div>
  {/if}
</div>