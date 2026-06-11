<script lang="ts">
  import { toastStore } from '../stores/toast';
</script>

<div class="fixed right-5 top-20 z-[120] flex w-[300px] flex-col gap-2">
  {#each $toastStore as toast}
    <div
      class={`rounded-[16px] border px-4 py-3 shadow-[0_10px_28px_rgba(0,0,0,0.12)] backdrop-blur-xl ${
        toast.type === 'success'
          ? 'border-slate-300/80 bg-[#eef4fa]/95 text-slate-900'
          : toast.type === 'error'
            ? 'border-red-200 bg-red-50/95 text-red-900'
            : 'border-slate-300/80 bg-[#eef4fa]/95 text-slate-900'
      }`}
    >
      <div class="flex items-center justify-between gap-3">
        <p class="text-sm font-medium leading-5">
          {toast.text}
        </p>

        <button
          type="button"
          on:click={() => toastStore.remove(toast.id)}
          class="shrink-0 text-lg leading-none text-slate-500 transition hover:text-slate-900"
        >
          ×
        </button>
      </div>
    </div>
  {/each}
</div>