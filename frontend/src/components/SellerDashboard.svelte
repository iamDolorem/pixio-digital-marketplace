<script lang="ts">
  import { onMount } from 'svelte';
  import { productsStore } from '../stores/products';
  import { userStore, type User } from '../stores/user';
  import type { Product } from '../types/product';
  import ProductCard from './ProductCard.svelte';
  import ConfirmModal from './ConfirmModal.svelte';

  let user: User | null = null;
  let products: Product[] = [];
  let productIdToDelete: number | null = null;

  onMount(() => {
    userStore.load();
    productsStore.load();

    const currentUser = userStore.getCurrentUser();

    if (!currentUser) {
      window.location.href = '/login?redirect=/seller/dashboard';
      return;
    }

    if (currentUser.role !== 'seller') {
      window.location.href = '/';
      return;
    }

    const unsubscribeUser = userStore.subscribe((value) => {
      user = value;
    });

    const unsubscribeProducts = productsStore.subscribe((value: Product[]) => {
      products = value.filter((product) => product.sellerId === currentUser.id);
    });

    return () => {
      unsubscribeUser();
      unsubscribeProducts();
    };
  });

  const askDeleteItem = (id: number) => {
    productIdToDelete = id;
  };

  const confirmDeleteItem = () => {
    if (productIdToDelete === null) return;

    productsStore.deleteProduct(productIdToDelete);
    productIdToDelete = null;
  };

  const cancelDeleteItem = () => {
    productIdToDelete = null;
  };
</script>

<div class="grid gap-x-5 gap-y-8 sm:grid-cols-2 lg:grid-cols-3">
  {#each products as product}
    <ProductCard
      product={product}
      href={`/product/view?id=${product.id}&from=seller`}
      showActions={true}
      editHref={`/seller/products/edit?id=${product.id}`}
      onDelete={() => askDeleteItem(product.id)}
    />
  {/each}
</div>

{#if productIdToDelete !== null}
  <ConfirmModal
    title="Удалить товар?"
    text="Товар будет удалён из списка продавца и перестанет отображаться в каталоге"
    confirmText="Удалить"
    cancelText="Отмена"
    onConfirm={confirmDeleteItem}
    onCancel={cancelDeleteItem}
  />
{/if}