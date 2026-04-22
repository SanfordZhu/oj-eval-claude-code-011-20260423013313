#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {

template<typename T, class Compare = std::less<T>>
class priority_queue {
	struct Node {
		T val;
		Node *left;
		Node *right;
		int dist;
		Node(const T &v) : val(v), left(nullptr), right(nullptr), dist(1) {}
	};
	struct MergeResult {
		Node *root;
		int dist;
		MergeResult(Node *r = nullptr, int d = 0) : root(r), dist(d) {}
	};

	Node *rt = nullptr;
	size_t sz = 0;
	Compare cmp;

	static int dist_of(Node *x) { return x ? x->dist : 0; }

	MergeResult merge_safe(Node *a, Node *b) {
		if (!a) return MergeResult(b, dist_of(b));
		if (!b) return MergeResult(a, dist_of(a));
		Node *stack[128];
		int top = 0;
		for (;;) {
			bool swapab = false;
			try { swapab = cmp(a->val, b->val); }
			catch (...) { throw runtime_error(); }
			if (swapab) { Node *t = a; a = b; b = t; }
			stack[top++] = a;
			if (a->right == nullptr) { break; }
			a = a->right;
		}
		Node *res = b;
		int rdist = dist_of(b);
		while (top) {
			Node *cur = stack[--top];
			Node *nl = cur->left;
			Node *nr = res;
			int ld = dist_of(nl);
			int rd = rdist;
			if (ld < rd) { Node *t = nl; nl = nr; nr = t; int td = ld; ld = rd; rd = td; }
			int nd = 1 + (ld < rd ? ld : rd);
			cur->left = nl;
			cur->right = nr;
			cur->dist = nd;
			res = cur;
			rdist = nd;
		}
		return MergeResult(res, rdist);
	}

	static void clear(Node *x) {
		if (!x) return;
		clear(x->left);
		clear(x->right);
		delete x;
	}
	static Node *clone(Node *x) {
		if (!x) return nullptr;
		Node *n = new Node(x->val);
		n->left = clone(x->left);
		n->right = clone(x->right);
		n->dist = x->dist;
		return n;
	}

public:
	priority_queue() {}
	priority_queue(const priority_queue &other) { rt = clone(other.rt); sz = other.sz; cmp = other.cmp; }
	~priority_queue() { clear(rt); }

	priority_queue &operator=(const priority_queue &other) {
		if (this == &other) return *this;
		Node *newrt = clone(other.rt);
		size_t newsz = other.sz;
		clear(rt);
		rt = newrt;
		sz = newsz;
		cmp = other.cmp;
		return *this;
	}

	const T & top() const { if (empty()) throw container_is_empty(); return rt->val; }

	void push(const T &e) {
		Node *n = new Node(e);
		try {
			MergeResult mr = merge_safe(rt, n);
			rt = mr.root;
			++sz;
		} catch (...) {
			delete n;
			throw runtime_error();
		}
	}

	void pop() {
		if (empty()) throw container_is_empty();
		Node *old = rt;
		try {
			MergeResult mr = merge_safe(rt->left, rt->right);
			rt = mr.root;
			delete old;
			--sz;
		} catch (...) {
			throw runtime_error();
		}
	}

	size_t size() const { return sz; }
	bool empty() const { return sz == 0; }

	void merge(priority_queue &other) {
		if (this == &other) return;
		try {
			MergeResult mr = merge_safe(rt, other.rt);
			rt = mr.root;
			sz += other.sz;
			other.rt = nullptr;
			other.sz = 0;
		} catch (...) {
			throw runtime_error();
		}
	}
};

}

#endif
