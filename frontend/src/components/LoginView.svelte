<script lang="ts">
  import { userStore } from '../stores/user';

  let email = '';
  let password = '';
  let error = '';

  const getRedirect = () => {
    const params = new URLSearchParams(window.location.search);
    return params.get('redirect');
  };

  const submit = async () => {
    error = '';

    const success = await userStore.login(email, password);

    if (!success) {
      error = 'Неверный email или пароль';
      return;
    }

    const redirect = getRedirect();
    const user = userStore.getCurrentUser();

    if (redirect === '/cart' && user?.role === 'seller') {
      userStore.logout();
      error = 'Корзина доступна только покупателю. Войдите как покупатель.';
      return;
    }

    if (redirect) {
      window.location.href = redirect;
      return;
    }

    window.location.href = user?.role === 'seller' ? '/seller/dashboard' : '/';
  };
</script>

<div class="rounded-[24px] border border-slate-300/80 bg-[rgba(255,255,255,0.55)] px-6 py-6">
  <h1 class="mb-2 text-3xl font-semibold text-slate-900">Вход</h1>

  <p class="mb-6 text-sm leading-6 text-slate-600">
    Введите данные тестового аккаунта.
  </p>

  <form
    class="space-y-4"
    autocomplete="on"
    on:submit|preventDefault={submit}
  >
    <div>
      <label class="mb-1 block text-sm text-slate-500" for="email">
        Email
      </label>
      <input
        id="email"
        name="pixio-login-email"
        bind:value={email}
        type="email"
        autocomplete="email"
        class="w-full rounded-[16px] border border-slate-300/80 bg-[rgba(255,255,255,0.72)] px-4 py-3 text-[14px] text-slate-900 outline-none focus:border-slate-500"
        placeholder="Введите email"
      />
    </div>

    <div>
      <label class="mb-1 block text-sm text-slate-500" for="password">
        Пароль
      </label>
      <input
        id="password"
        name="pixio-login-password"
        bind:value={password}
        type="password"
        autocomplete="new-password"
        class="w-full rounded-[16px] border border-slate-300/80 bg-[rgba(255,255,255,0.72)] px-4 py-3 text-[14px] text-slate-900 outline-none focus:border-slate-500"
        placeholder="Введите пароль"
      />
    </div>

    <div class="text-right">
      <a
        href="/forgot-password"
        class="text-sm text-slate-500 transition hover:text-slate-900 hover:underline"
      >
        Забыли пароль?
      </a>
    </div>

    {#if error}
      <p class="text-sm font-medium text-red-500">{error}</p>
    {/if}

    <button
      type="submit"
      class="w-full rounded-[16px] bg-[#2d4157] px-5 py-3 text-sm font-medium text-white transition hover:bg-[#24364a]"
    >
      Войти
    </button>
  </form>

  <p class="mt-5 text-center text-sm text-slate-600">
    Нет аккаунта?
    <a href="/register" class="font-medium text-slate-900 hover:underline">
      Зарегистрироваться
    </a>
  </p>
</div>