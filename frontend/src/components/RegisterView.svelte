<script lang="ts">
  import { userStore, type UserRole } from '../stores/user';

  let name = '';
  let email = '';
  let password = '';
  const role: UserRole = 'buyer';
  let error = '';
  let confirmPassword = '';

  const submit = async () => {
    error = '';

    if (!name.trim() || !email.trim() || !password.trim() || !confirmPassword.trim()) {
      error = 'Заполните все поля';
      return;
    }

    if (password.length < 6) {
      error = 'Пароль должен быть не короче 6 символов';
      return;
    }

    if (password !== confirmPassword) {
      error = 'Пароли не совпадают';
      return;
    }

    const result = await userStore.register(name, email, password, role);

    if (!result.success) {
      error = result.message;
      return;
    }

    window.location.href = '/';
  };
</script>

<div class="rounded-[24px] border border-slate-300/80 bg-[rgba(255,255,255,0.55)] px-6 py-6">
  <h1 class="mb-2 text-3xl font-semibold text-slate-900">Регистрация</h1>

  <form class="space-y-4" on:submit|preventDefault={submit}>
    <div>
      <label class="mb-1 block text-sm text-slate-500" for="name">
        Имя
      </label>
      <input
        id="name"
        bind:value={name}
        type="text"
        class="w-full rounded-[16px] border border-slate-300/80 bg-[rgba(255,255,255,0.72)] px-4 py-3 text-[14px] text-slate-900 outline-none focus:border-slate-500"
        placeholder="Введите имя"
      />
    </div>

    <div>
      <label class="mb-1 block text-sm text-slate-500" for="email">
        Email
      </label>
      <input
        id="email"
        bind:value={email}
        type="email"
        autocomplete="email"
        class="w-full rounded-[16px] border border-slate-300/80 bg-[rgba(255,255,255,0.72)] px-4 py-3 text-[14px] text-slate-900 outline-none focus:border-slate-500"
        placeholder="name@example.com"
      />
    </div>

    <div>
      <label class="mb-1 block text-sm text-slate-500" for="password">
        Пароль
      </label>
      <input
        id="password"
        bind:value={password}
        type="password"
        autocomplete="new-password"
        class="w-full rounded-[16px] border border-slate-300/80 bg-[rgba(255,255,255,0.72)] px-4 py-3 text-[14px] text-slate-900 outline-none focus:border-slate-500"
        placeholder="Минимум 6 символов"
      />
    </div>

    <div>
      <label class="mb-1 block text-sm text-slate-500" for="confirm-password">
        Повторите пароль
      </label>
      <input
        id="confirm-password"
        bind:value={confirmPassword}
        type="password"
        autocomplete="new-password"
        class="w-full rounded-[16px] border border-slate-300/80 bg-[rgba(255,255,255,0.72)] px-4 py-3 text-[14px] text-slate-900 outline-none focus:border-slate-500"
        placeholder="Повторите пароль"
      />
    </div>

    {#if error}
      <p class="text-sm font-medium text-red-500">{error}</p>
    {/if}

    <button
      type="submit"
      class="w-full rounded-[16px] bg-[#2d4157] px-5 py-3 text-sm font-medium text-white transition hover:bg-[#24364a]"
    >
      Зарегистрироваться
    </button>
  </form>

  <div class="mt-5 rounded-[18px] bg-[rgba(255,255,255,0.48)] px-4 py-4 text-sm leading-6 text-slate-600">
    <p class="mb-1 font-medium text-slate-900">Вы продавец?</p>
    <p>
      Для регистрации продавца напишите администрации платформы:
      <span class="font-medium text-slate-900">seller@pixio.local</span>
    </p>
  </div>

  <p class="mt-5 text-center text-sm text-slate-600">
    Уже есть аккаунт?
    <a href="/login" class="font-medium text-slate-900 hover:underline">
      Войти
    </a>
  </p>
</div>