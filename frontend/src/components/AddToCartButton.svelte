<script lang="ts">
  import { onMount } from 'svelte';
  import { cartStore } from '../stores/cart';
  import { toastStore } from '../stores/toast';

  export let id: number;
  export let title: string;
  export let price: number;
  export let category: string;
  export let image: string;

  let inCart = false;

  onMount(() => {
    cartStore.load();

    const unsubscribe = cartStore.subscribe((items) => {
      inCart = items.some((item) => item.productId === id);
    });

    return () => {
      unsubscribe();
    };
  });

  const handleClick = () => {
    if (inCart) return;

    cartStore.addToCart({
      productId: id,
      title,
      price,
      category,
      image
    });

    toastStore.show('Товар добавлен в корзину', 'success');
  };
</script>

<button
  on:click={handleClick}
  class={`rounded-[16px] px-5 py-3 text-sm font-medium transition ${
    inCart
      ? 'bg-[rgba(45,65,87,0.08)] text-slate-900'
      : 'bg-[#2d4157] text-white hover:bg-[#24364a]'
  }`}
>
  {inCart ? 'Уже в корзине' : 'Добавить в корзину'}
</button>