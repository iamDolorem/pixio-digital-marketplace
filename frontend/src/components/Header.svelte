<script lang="ts">
  import { onMount } from 'svelte';
  import { cartStore } from '../stores/cart';
  import { userStore, type User } from '../stores/user';

  let isScrolled = false;
  let cartCount = 0;
  let user: User | null = null;
  let mobileMenuOpen = false;

  onMount(() => {
    cartStore.load();
    userStore.load();

    const unsubscribeCart = cartStore.subscribe((items) => {
      cartCount = items.length;
    });

    const unsubscribeUser = userStore.subscribe((value) => {
      user = value;
    });

    const handleScroll = () => {
      isScrolled = window.scrollY > 20;
    };

    const handleResize = () => {
      if (window.innerWidth >= 768) {
        mobileMenuOpen = false;
      }
    };

    handleScroll();
    window.addEventListener('scroll', handleScroll);
    window.addEventListener('resize', handleResize);

    return () => {
      unsubscribeCart();
      unsubscribeUser();
      window.removeEventListener('scroll', handleScroll);
      window.removeEventListener('resize', handleResize);
    };
  });

  const logout = () => {
    userStore.logout();
    mobileMenuOpen = false;
    window.location.href = '/';
  };

  const closeMobileMenu = () => {
    mobileMenuOpen = false;
  };

  const navigateTo = (path: string) => {
  mobileMenuOpen = false;
  window.location.assign(path);
};
</script>

<header
  class={`sticky top-0 z-50 border-b transition-all duration-300 ${
    isScrolled
      ? 'border-[#41566d] bg-[rgba(35,52,73,0.86)] backdrop-blur-xl shadow-[0_6px_24px_rgba(0,0,0,0.10)]'
      : 'border-transparent bg-[#2d4157]'
  }`}
>
  <div class="mx-auto flex max-w-6xl items-center justify-between px-5 py-2.5">
    <a href="/" class="flex min-w-0 items-center gap-2 text-[#f4f8fc] sm:gap-3">
      <svg
        viewBox="0 0 32 32"
        class="h-6 w-6 shrink-0 sm:h-7 sm:w-7"
        fill="currentColor"
        xmlns="http://www.w3.org/2000/svg"
        aria-hidden="true"
      >
        <rect x="4" y="4" width="8" height="8" rx="1.5" />
        <rect x="12" y="12" width="8" height="8" rx="1.5" />
        <rect x="20" y="4" width="8" height="8" rx="1.5" />
        <rect x="20" y="20" width="8" height="8" rx="1.5" />
      </svg>

      <span class="text-[1.45rem] font-black uppercase leading-none tracking-[0.08em] sm:text-[2.1rem]">
        PIXIO
      </span>
    </a>

    <nav class="hidden items-center gap-6 text-[0.95rem] font-semibold uppercase tracking-[0.04em] text-[#f4f8fc] md:flex">
      <a href="/" class="nav-link">Каталог</a>

      {#if user?.role !== 'seller'}
        <a href="/cart" class="nav-link inline-flex items-center gap-2">
          <span>Корзина</span>

          {#if cartCount > 0}
            <span class="rounded-full bg-[#f4f8fc] px-2 py-0.5 text-xs font-bold text-[#2d4157]">
              {cartCount}
            </span>
          {/if}
        </a>
      {/if}

      {#if user?.role === 'seller'}
        <a href="/seller/dashboard" class="nav-link">Мои товары</a>
      {/if}

      {#if user}
        <a href="/profile" class="nav-link">Профиль</a>
      {/if}
    </nav>

    <div class="flex items-center gap-2">
      {#if user}
        <a
          href="/"
          on:click|preventDefault={logout}
          class="hidden h-9 w-[86px] items-center justify-center rounded-[14px] border border-[#6c8399] bg-[#3b556f] p-0 text-sm font-semibold leading-none text-white transition hover:bg-[#486785] sm:inline-flex"
        >
          Выйти
        </a>
      {:else}
        <a
          href="/login"
          class="hidden h-9 w-[86px] items-center justify-center rounded-[14px] border border-[#6c8399] bg-[#3b556f] p-0 text-sm font-semibold leading-none text-white transition hover:bg-[#486785] sm:inline-flex"
        >
          Войти
        </a>
      {/if}

      <button
        type="button"
        class="inline-flex h-10 w-10 items-center justify-center rounded-[14px] border border-[#6c8399] bg-[#3b556f] text-white transition hover:bg-[#486785] md:hidden"
        aria-label={mobileMenuOpen ? 'Закрыть меню' : 'Открыть меню'}
        aria-expanded={mobileMenuOpen}
        on:click={() => (mobileMenuOpen = !mobileMenuOpen)}
      >
        {#if mobileMenuOpen}
          <svg class="h-5 w-5" viewBox="0 0 24 24" fill="none" aria-hidden="true">
            <path
              d="M6 6l12 12M18 6L6 18"
              stroke="currentColor"
              stroke-width="2"
              stroke-linecap="round"
            />
          </svg>
        {:else}
          <svg class="h-5 w-5" viewBox="0 0 24 24" fill="none" aria-hidden="true">
            <path
              d="M4 7h16M4 12h16M4 17h16"
              stroke="currentColor"
              stroke-width="2"
              stroke-linecap="round"
            />
          </svg>
        {/if}
      </button>
    </div>
  </div>

  {#if mobileMenuOpen}
    <div class="border-t border-[#41566d] bg-[#2d4157] px-4 py-4 shadow-[0_10px_24px_rgba(0,0,0,0.16)] md:hidden">
      <nav class="mx-auto flex max-w-6xl flex-col gap-2 text-sm font-semibold uppercase tracking-[0.04em] text-[#f4f8fc]">
        <a href="/" class="mobile-nav-link" on:click={closeMobileMenu}>
          Каталог
        </a>

        {#if user?.role !== 'seller'}
          <a href="/cart" class="mobile-nav-link" on:click={closeMobileMenu}>
            <span>Корзина</span>

            {#if cartCount > 0}
              <span class="rounded-full bg-[#f4f8fc] px-2 py-0.5 text-xs font-bold text-[#2d4157]">
                {cartCount}
              </span>
            {/if}
          </a>
        {/if}

        {#if user?.role === 'seller'}
          <a href="/seller/dashboard" class="mobile-nav-link" on:click={closeMobileMenu}>
            Мои товары
          </a>
        {/if}

        {#if user}
          <a href="/profile" class="mobile-nav-link" on:click={closeMobileMenu}>
            Профиль
          </a>

          <button
            type="button"
            on:click={logout}
            class="mobile-nav-link mt-2 w-full border border-[#6c8399] bg-[#3b556f] text-left text-[#f4f8fc] hover:bg-[#486785]"
          >
            <span>Выйти</span>

            <svg class="h-5 w-5 opacity-80" viewBox="0 0 24 24" fill="none" aria-hidden="true">
              <path
                d="M10 17l5-5-5-5M15 12H3M21 4v16"
                stroke="currentColor"
                stroke-width="2"
                stroke-linecap="round"
                stroke-linejoin="round"
              />
            </svg>
          </button>
        {:else}
          <a href="/login" class="mobile-nav-link" on:click={closeMobileMenu}>
            Войти
          </a>
        {/if}
      </nav>
    </div>
  {/if}
</header>

<style>
  .nav-link {
    position: relative;
    padding-bottom: 4px;
    transition: color 0.2s ease;
  }

  .nav-link::after {
    content: '';
    position: absolute;
    left: 50%;
    bottom: 0;
    width: 0;
    height: 2px;
    background: #f4f8fc;
    transform: translateX(-50%);
    transition: width 0.25s ease;
  }

  .nav-link:hover::after {
    width: 100%;
  }

  .mobile-nav-link {
    display: flex;
    align-items: center;
    justify-content: space-between;
    border-radius: 14px;
    padding: 12px 14px;
    transition: background 0.2s ease;
  }

  .mobile-nav-link:hover {
    background: rgba(255, 255, 255, 0.08);
  }
</style>