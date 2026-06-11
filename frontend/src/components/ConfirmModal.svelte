<script lang="ts">
  export let title = 'Подтвердите действие';
  export let text = '';
  export let confirmText = 'Подтвердить';
  export let cancelText = 'Отмена';
  export let onConfirm: () => void;
  export let onCancel: () => void;
</script>

<div class="fixed inset-0 z-[100] flex items-center justify-center bg-slate-950/45 px-5 backdrop-blur-sm">
  <div class="w-full max-w-md rounded-[24px] border border-slate-300/80 bg-[#eef4fa] px-6 py-6 shadow-[0_18px_60px_rgba(0,0,0,0.18)]">
    <h2 class="mb-2 text-2xl font-semibold text-slate-900">{title}</h2>
    <p class="mb-6 text-sm leading-6 text-slate-600">{text}</p>

    <div class="flex justify-end gap-3">
      <button
        type="button"
        on:click={onCancel}
        class="rounded-[16px] bg-[rgba(45,65,87,0.08)] px-5 py-2.5 text-sm text-slate-900 transition hover:bg-[rgba(45,65,87,0.14)]"
      >
        {cancelText}
      </button>

      <button
        type="button"
        on:click={onConfirm}
        class="rounded-[16px] bg-[#2d4157] px-5 py-2.5 text-sm font-medium text-white transition hover:bg-[#24364a]"
      >
        {confirmText}
      </button>
    </div>
  </div>
</div>