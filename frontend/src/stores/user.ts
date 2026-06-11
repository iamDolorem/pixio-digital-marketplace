import { writable, get } from 'svelte/store';

export type UserRole = 'buyer' | 'seller';

export type User = {
  id: number;
  name: string;
  email: string;
  role: UserRole;
};

const STORAGE_KEY = 'pixio-user';
const API_BASE = 'http://192.168.1.87:5555';

type StoredUser = User & {
  password: string;
};

const CURRENT_USER_KEY = 'pixio-user';
const USERS_KEY = 'pixio-users';

const demoUsers: StoredUser[] = [
  {
    id: 1,
    name: 'Продавец',
    email: 'seller@pixio.ru',
    password: 'seller123',
    role: 'seller'
  },
  {
    id: 2,
    name: 'Покупатель',
    email: 'buyer@pixio.ru',
    password: 'buyer123',
    role: 'buyer'
  }
];

const getUsers = () => {
  if (typeof localStorage === 'undefined') return demoUsers;

  const raw = localStorage.getItem(USERS_KEY);

  if (!raw) {
    localStorage.setItem(USERS_KEY, JSON.stringify(demoUsers));
    return demoUsers;
  }

  try {
    const parsed: StoredUser[] = JSON.parse(raw);
    return Array.isArray(parsed) ? parsed : demoUsers;
  } catch {
    return demoUsers;
  }
};

const saveUsers = (users: StoredUser[]) => {
  if (typeof localStorage === 'undefined') return;
  localStorage.setItem(USERS_KEY, JSON.stringify(users));
};

const toPublicUser = (user: StoredUser): User => ({
  id: user.id,
  name: user.name,
  email: user.email,
  role: user.role
});

function createUserStore() {
  const store = writable<User | null>(null);

  const load = () => {
    if (typeof localStorage === 'undefined') return;

    const raw = localStorage.getItem(CURRENT_USER_KEY);
    if (!raw) return;

    try {
      const parsed: User = JSON.parse(raw);
      store.set(parsed);
    } catch (error) {
      console.error('Failed to load user from localStorage', error);
    }
  };

  const login = async (email: string, password: string) => {
    try {
      const response = await fetch(`${API_BASE}/api/auth/login`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({
          email: email.trim(),
          password
        })
      });

      if (!response.ok) {
        return false;
      }

      const data = await response.json();

      if (!data.success || !data.user) {
        return false;
      }

      const user: User = {
        id: Number(data.user.id),
        name: String(data.user.name),
        email: String(data.user.email),
        role: data.user.role as UserRole
      };

      store.set(user);
      localStorage.setItem(CURRENT_USER_KEY, JSON.stringify(user));

      return true;
    } catch (error) {
      console.error('Failed to login through backend', error);
      return false;
    }
  };

  const register = async (
    name: string,
    email: string,
    password: string,
    role: UserRole
  ) => {
    try {
      const response = await fetch(`${API_BASE}/api/auth/register`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({
          name: name.trim(),
          email: email.trim(),
          password
        })
      });

      const data = await response.json();

      if (!response.ok || !data.success || !data.user) {
        return {
          success: false,
          message: data.message || 'Не удалось зарегистрироваться'
        };
      }

      const user: User = {
        id: Number(data.user.id),
        name: String(data.user.name),
        email: String(data.user.email),
        role: data.user.role as UserRole
      };

      store.set(user);
      localStorage.setItem(CURRENT_USER_KEY, JSON.stringify(user));

      return {
        success: true,
        message: ''
      };
    } catch (error) {
      console.error('Failed to register through backend', error);

      return {
        success: false,
        message: 'Ошибка соединения с сервером'
      };
    }
  };

  const updateProfile = async (name: string, email: string) => {
    const currentUser = get(store);

    if (!currentUser) {
      return false;
    }

    try {
      const response = await fetch(`${API_BASE}/api/users/${currentUser.id}`, {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({
          name: name.trim(),
          email: email.trim()
        })
      });

      const data = await response.json();

      if (!response.ok || !data.success || !data.user) {
        console.error('Failed to update profile', data);
        return false;
      }

      const updatedUser: User = {
        id: Number(data.user.id),
        name: String(data.user.name),
        email: String(data.user.email),
        role: data.user.role
      };

      store.set(updatedUser);

      if (typeof localStorage !== 'undefined') {
        localStorage.setItem(STORAGE_KEY, JSON.stringify(updatedUser));
      }

      return true;
    } catch (error) {
      console.error('Failed to update profile through backend', error);
      return false;
    }
  };

  const logout = () => {
    store.set(null);
    localStorage.removeItem(CURRENT_USER_KEY);
  };

  const getCurrentUser = () => {
    return get(store);
  };

  return {
    subscribe: store.subscribe,
    load,
    login,
    register,
    updateProfile,
    logout,
    getCurrentUser
  };
}

export const userStore = createUserStore();